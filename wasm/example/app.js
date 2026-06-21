import createSiloModule from "../../dist/wasm/silo_wasm.js";

const $ = (selector) => document.querySelector(selector);
const logEl = $("#log");
const resultEl = $("#result");
const preprocessButton = $("#preprocess");
const loadStateButton = $("#load-state");
const runQueryButton = $("#run-query");

let modulePromise;
let currentHandle = null;

preprocessButton.addEventListener("click", preprocessAndDownloadState);
loadStateButton.addEventListener("click", loadProcessedState);
runQueryButton.addEventListener("click", runQuery);

function log(message) {
    logEl.textContent += `${message}\n`;
}

function getSiloModule() {
    if (!modulePromise) {
        if (!crossOriginIsolated) {
            log("Warning: this page is not cross-origin isolated. Pthread-enabled WASM may not start.");
        }
        modulePromise = createSiloModule({
            print: (message) => log(message),
            printErr: (message) => log(`stderr: ${message}`),
        });
    }
    return modulePromise;
}

async function preprocessAndDownloadState() {
    const files = [...$("#input-files").files];
    const configPath = $("#config-path").value.trim();
    if (!files.length || !configPath) {
        log("Choose input files and a preprocessing config path first.");
        return;
    }

    await withDisabled(preprocessButton, async () => {
        const module = await getSiloModule();
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
        currentHandle = module.preprocess(configPath);

        log("Saving processed state...");
        module.save(currentHandle, "/example-output");
        const archive = readDirectoryAsArchive(module, "/example-output");
        downloadJson("silo-state.json", archive);
        log(`Downloaded ${archive.files.length} processed state file(s).`);
    });
}

async function loadProcessedState() {
    const file = $("#state-file").files[0];
    if (!file) {
        log("Choose a processed state JSON file first.");
        return;
    }

    await withDisabled(loadStateButton, async () => {
        const module = await getSiloModule();
        disposeCurrentHandle(module);

        removeTreeIfExists(module, "/loaded-state");
        mkdirp(module, "/loaded-state");

        const archive = JSON.parse(await file.text());
        writeArchiveToDirectory(module, archive, "/loaded-state");
        currentHandle = module.load("/loaded-state");
        log(`Loaded state. Database info: ${module.info(currentHandle)}`);
    });
}

async function runQuery() {
    const query = $("#query").value.trim();
    if (currentHandle === null) {
        log("Load or preprocess a state before querying.");
        return;
    }
    if (!query) {
        log("Enter a SaneQL query first.");
        return;
    }

    await withDisabled(runQueryButton, async () => {
        const module = await getSiloModule();
        resultEl.textContent = module.query(currentHandle, query);
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
        log(error?.stack || String(error));
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
