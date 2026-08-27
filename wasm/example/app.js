import createRhydbModule from "../dist/rhydb_wasm.js";

const $ = (selector) => document.querySelector(selector);
const logEl = $("#log");
const resultEl = $("#result");
const preprocessButton = $("#preprocess");
const loadStateButton = $("#load-state");
const runQueryButton = $("#run-query");
const clearLogButton = $("#clear-log");

let modulePromise;
let loadedModule = null;
let currentHandle = null;

installGlobalErrorLogging();

preprocessButton.addEventListener("click", preprocessAndDownloadState);
loadStateButton.addEventListener("click", loadProcessedState);
runQueryButton.addEventListener("click", runQuery);
clearLogButton.addEventListener("click", () => logEl.replaceChildren());

// --- Logging -------------------------------------------------------------
// rhydb writes its spdlog output to stdout/stderr. Emscripten routes those
// through Module.print/Module.printErr (worker threads proxy their output to
// the main thread), so wiring those two options is what puts the C++ log into
// this page. Everything else here catches the failures that would otherwise
// only ever reach the devtools console.

const MAX_LOG_LINES = 5000;
// spdlog's stdout sink colours its output because Emscripten reports stdout as
// a TTY, so the raw SGR escapes have to be stripped before rendering as text.
const ANSI_SGR = new RegExp("\\u001b\\[[0-9;]*m", "g");

function log(message, level = "info") {
    const text = String(message).replace(ANSI_SGR, "");
    for (const line of text.split("\n")) {
        const lineEl = document.createElement("span");
        lineEl.className = `log-line log-${level}`;
        lineEl.textContent = `${line}\n`;
        logEl.appendChild(lineEl);
    }
    while (logEl.childElementCount > MAX_LOG_LINES) {
        logEl.removeChild(logEl.firstElementChild);
    }
    logEl.scrollTop = logEl.scrollHeight;
}

function installGlobalErrorLogging() {
    window.addEventListener("error", (event) => {
        const description = event.error
            ? describeError(event.error)
            : `${event.message} (${event.filename}:${event.lineno})`;
        log(description, "error");
    });
    window.addEventListener("unhandledrejection", (event) => {
        log(describeError(event.reason), "error");
    });

    // Emscripten's own diagnostics (aborts, pthread startup failures, worker
    // errors) go to the console, not through print/printErr. Mirror the console
    // so they land in the page too.
    const mirrored = [
        ["log", "info"],
        ["info", "info"],
        ["warn", "warn"],
        ["error", "error"],
    ];
    for (const [method, level] of mirrored) {
        const original = console[method].bind(console);
        console[method] = (...args) => {
            original(...args);
            log(args.map(formatConsoleArgument).join(" "), level);
        };
    }
}

function formatConsoleArgument(value) {
    if (typeof value === "string") return value;
    if (value instanceof Error) return value.stack || `${value.name}: ${value.message}`;
    try {
        return JSON.stringify(value);
    } catch {
        return String(value);
    }
}

// A C++ exception that escapes an embind function does not arrive as an Error.
// With Emscripten's JS exception model it is an opaque `CppException` wrapper
// whose only field is `excPtr`, the address of the C++ exception object in the
// wasm heap, so stringifying it yields "[object Object]" and JSON.stringify
// yields {"excPtr":...}. The getExceptionMessage runtime method exported in
// wasm/CMakeLists.txt turns it into [type, message].
function describeError(error) {
    if (error instanceof Error) {
        return error.stack || `${error.name}: ${error.message}`;
    }
    const decoded = decodeWasmException(error);
    if (decoded) return decoded;
    if (error?.excPtr !== undefined) {
        return `Uncaught C++ exception at ${error.excPtr}; its message could not be decoded.`;
    }
    return formatConsoleArgument(error);
}

// Anything that is not an Error may be a wasm exception, so hand it to
// getExceptionMessage unconditionally and fall back if it cannot decode it.
function decodeWasmException(error) {
    if (!loadedModule?.getExceptionMessage) return null;
    try {
        const message = loadedModule.getExceptionMessage(error);
        if (Array.isArray(message)) {
            const text = message.filter(Boolean).join(": ");
            return text.length > 0 ? text : null;
        }
        return message ? String(message) : null;
    } catch {
        return null;
    }
}

// The embind calls below run synchronously on the main thread, so the browser
// cannot repaint until they return. Yielding first makes the preceding log
// lines visible instead of appearing all at once after the call finishes.
function flushLog() {
    return new Promise((resolve) => requestAnimationFrame(() => setTimeout(resolve, 0)));
}

// --- Module --------------------------------------------------------------

function getRhydbModule() {
    if (!modulePromise) {
        if (!crossOriginIsolated) {
            log(
                "Warning: this page is not cross-origin isolated. Pthread-enabled WASM may not start.",
                "warn",
            );
        }
        log("Loading rhydb WASM module...");
        modulePromise = createRhydbModule({
            print: (message) => log(message, "stdout"),
            printErr: (message) => log(message, "stderr"),
        }).then(
            (module) => {
                loadedModule = module;
                log("rhydb WASM module ready.");
                return module;
            },
            (error) => {
                // Drop the cached promise so a failed init is retryable.
                modulePromise = undefined;
                log(describeError(error), "error");
                throw error;
            },
        );
    }
    return modulePromise;
}

async function preprocessAndDownloadState() {
    const files = [...$("#input-files").files];
    const configPath = $("#config-path").value.trim();
    if (!files.length || !configPath) {
        log("Choose input files and a preprocessing config path first.", "warn");
        return;
    }

    await withDisabled(preprocessButton, async () => {
        const module = await getRhydbModule();
        disposeCurrentHandle(module);

        removeTreeIfExists(module, "/example-input");
        removeTreeIfExists(module, "/example-output");
        mkdirp(module, "/example-input");

        for (const file of files) {
            const relativePath = file.webkitRelativePath || file.name;
            const path = `/example-input/${relativePath}`;
            mkdirp(module, path.split("/").slice(0, -1).join("/"));
            module.FS.writeFile(path, new Uint8Array(await file.arrayBuffer()));
        }

        log("Preprocessing uploaded files...");
        module.FS.chdir("/example-input");
        await flushLog();
        currentHandle = module.preprocess(configPath);

        log("Saving processed state...");
        await flushLog();
        module.save(currentHandle, "/example-output");
        const archive = readDirectoryAsArchive(module, "/example-output");
        downloadJson("silo-state.json", archive);
        log(`Downloaded ${archive.files.length} processed state file(s).`);
    });
}

async function loadProcessedState() {
    const file = $("#state-file").files[0];
    if (!file) {
        log("Choose a processed state JSON file first.", "warn");
        return;
    }

    await withDisabled(loadStateButton, async () => {
        const module = await getRhydbModule();
        disposeCurrentHandle(module);

        removeTreeIfExists(module, "/loaded-state");
        mkdirp(module, "/loaded-state");

        const archive = JSON.parse(await file.text());
        writeArchiveToDirectory(module, archive, "/loaded-state");
        log("Loading processed state...");
        await flushLog();
        currentHandle = module.load("/loaded-state");
        log(`Loaded state. Database info: ${module.info(currentHandle)}`);
    });
}

async function runQuery() {
    const query = $("#query").value.trim();
    if (currentHandle === null) {
        log("Load or preprocess a state before querying.", "warn");
        return;
    }
    if (!query) {
        log("Enter a SaneQL query first.", "warn");
        return;
    }

    await withDisabled(runQueryButton, async () => {
        const module = await getRhydbModule();
        log("Running query...");
        await flushLog();
        resultEl.textContent = module.query(currentHandle, query);
        log("Query finished.");
    });
}

function disposeCurrentHandle(module) {
    if (currentHandle !== null) {
        module.dispose(currentHandle);
        currentHandle = null;
    }
}

async function withDisabled(button, fn) {
    button.disabled = true;
    resultEl.textContent = "";
    try {
        await fn();
    } catch (error) {
        log(describeError(error), "error");
    } finally {
        button.disabled = false;
    }
}

function readDirectoryAsArchive(module, rootPath) {
    const files = [];
    walk(rootPath);
    return { format: "silo-wasm-state-v1", files };

    function walk(directory) {
        for (const entry of module.FS.readdir(directory)) {
            if (entry === "." || entry === "..") continue;
            const path = `${directory}/${entry}`;
            const relativePath = path.slice(rootPath.length + 1);
            if (isDirectory(module, path)) {
                walk(path);
            } else {
                files.push({
                    path: relativePath,
                    base64: bytesToBase64(module.FS.readFile(path)),
                });
            }
        }
    }
}

function writeArchiveToDirectory(module, archive, rootPath) {
    if (archive.format !== "silo-wasm-state-v1" || !Array.isArray(archive.files)) {
        throw new Error("Not a SILO WASM state archive.");
    }
    for (const file of archive.files) {
        const path = `${rootPath}/${file.path}`;
        mkdirp(module, path.split("/").slice(0, -1).join("/"));
        module.FS.writeFile(path, base64ToBytes(file.base64));
    }
}

function mkdirp(module, path) {
    const parts = path.split("/").filter(Boolean);
    let current = "";
    for (const part of parts) {
        current += `/${part}`;
        if (!module.FS.analyzePath(current).exists) module.FS.mkdir(current);
    }
}

function removeTreeIfExists(module, path) {
    if (!module.FS.analyzePath(path).exists) return;
    removeTree(module, path);
}

function removeTree(module, path) {
    for (const entry of module.FS.readdir(path)) {
        if (entry === "." || entry === "..") continue;
        const child = `${path}/${entry}`;
        if (isDirectory(module, child)) {
            removeTree(module, child);
        } else {
            module.FS.unlink(child);
        }
    }
    module.FS.rmdir(path);
}

function isDirectory(module, path) {
    try {
        module.FS.readdir(path);
        return true;
    } catch {
        return false;
    }
}

function downloadJson(filename, value) {
    const blob = new Blob([JSON.stringify(value, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = filename;
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 0);
}

function bytesToBase64(bytes) {
    let binary = "";
    for (let i = 0; i < bytes.length; i += 0x8000) {
        binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
    }
    return btoa(binary);
}

function base64ToBytes(base64) {
    const binary = atob(base64);
    const bytes = new Uint8Array(binary.length);
    for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
    return bytes;
}
