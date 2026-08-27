import createRhydbModule from "../dist/rhydb_wasm.js";

const $ = (selector) => document.querySelector(selector);
const logEl = $("#log");
const resultEl = $("#result");
const preprocessButton = $("#preprocess");
const preprocessBamButton = $("#preprocess-bam");
const preprocessFastaButton = $("#preprocess-fasta");
const loadStateButton = $("#load-state");
const runQueryButton = $("#run-query");

let modulePromise;
let currentHandle = null;
// Last result of the "Reads around this position" explorer, kept for pagination.
let readsState = null;

preprocessButton.addEventListener("click", () =>
    preprocessAndDownloadState({
        button: preprocessButton,
        filesSelector: "#input-files",
        configSelector: "#config-path",
        method: "preprocess",
    }),
);
preprocessBamButton.addEventListener("click", () =>
    preprocessAndDownloadState({
        button: preprocessBamButton,
        filesSelector: "#bam-input-files",
        configSelector: "#bam-config-path",
        method: "preprocessBam",
    }),
);
preprocessFastaButton.addEventListener("click", () =>
    preprocessAndDownloadState({
        button: preprocessFastaButton,
        filesSelector: "#fasta-input-files",
        configSelector: "#fasta-config-path",
        method: "preprocessFasta",
    }),
);
loadStateButton.addEventListener("click", loadProcessedState);
runQueryButton.addEventListener("click", runQuery);
$("#reads-explore").addEventListener("click", exploreReads);
$("#reads-min").addEventListener("change", () => {
    if (readsState) {
        readsState.page = 0;
        renderReads();
    }
});

function log(message) {
    logEl.textContent += `${message}\n`;
}

function getRhydbModule() {
    if (!modulePromise) {
        if (!crossOriginIsolated) {
            log("Warning: this page is not cross-origin isolated. Pthread-enabled WASM may not start.");
        }
        modulePromise = createRhydbModule({
            print: (message) => log(message),
            printErr: (message) => log(`stderr: ${message}`),
        });
    }
    return modulePromise;
}

// Shared by the NDJSON and BAM flows: they differ only in the input elements and
// in which module entry point (`preprocess` vs `preprocessBam`) is invoked.
async function preprocessAndDownloadState({ button, filesSelector, configSelector, method }) {
    const files = [...$(filesSelector).files];
    const configPath = $(configSelector).value.trim();
    if (!files.length || !configPath) {
        log("Choose input files and a preprocessing config path first.");
        return;
    }

    await withDisabled(button, async () => {
        const module = await getRhydbModule();
        if (typeof module[method] !== "function") {
            log(`This WASM build does not expose ${method}(). Rebuild the module (make build/wasm/...).`);
            return;
        }
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

        log(`Preprocessing uploaded files with ${method}()...`);
        module.FS.chdir("/example-input");
        currentHandle = module[method](configPath);

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
        const module = await getRhydbModule();
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
        const module = await getRhydbModule();
        resultEl.textContent = module.query(currentHandle, query);
    });
}

// The nucleotide alphabet and the Paul-Tol color palette, mirroring the source
// component: a symbol's color is chosen by its index in the alphabet.
const READS_ALPHABET = ["A", "C", "G", "T", "-"];
const READS_PALETTE = [
    "rgb(51, 34, 136)",
    "rgb(17, 119, 51)",
    "rgb(136, 204, 238)",
    "rgb(68, 170, 153)",
    "rgb(153, 153, 51)",
    "rgb(221, 204, 119)",
    "rgb(204, 102, 119)",
    "rgb(136, 34, 85)",
    "rgb(170, 68, 153)",
];
// Positions a read did not reach reconstruct to the missing symbol. Empty is a
// defensive fallback for positions past the end of the reference.
const READS_MISSING = "N";
const READS_PAGE_SIZE = 20;

function readsColor(symbol) {
    const index = READS_ALPHABET.indexOf(symbol);
    return READS_PALETTE[(index < 0 ? READS_ALPHABET.length : index) % READS_PALETTE.length];
}

function isCovered(symbol) {
    return symbol !== "" && symbol !== READS_MISSING;
}

// Build the read-pattern query: one projected column per position in the window,
// grouped into distinct base patterns with a count. Each row of the table is one
// read/sequence, so a group is a pattern shared by `n` reads.
function readPatternQuery(table, seqColumn, positions) {
    const maps = positions.map((p, i) => `p${i + 1} := ${seqColumn}.at(${p})`).join(", ");
    const groupCols = positions.map((_, i) => `p${i + 1}`).join(", ");
    return `${table}.map({${maps}}).groupBy({n := count()}, {${groupCols}})`;
}

async function exploreReads() {
    if (currentHandle === null) {
        log("Load or preprocess a database before exploring reads.");
        return;
    }
    const table = $("#reads-table").value.trim();
    const seqColumn = $("#reads-seqcol").value.trim();
    const center = Number($("#reads-pos").value);
    const width = Number($("#reads-window").value);
    if (!table || !seqColumn || !Number.isInteger(center) || center < 1) {
        log("Enter a table, a sequence column, and a 1-based position first.");
        return;
    }

    await withDisabled($("#reads-explore"), async () => {
        const module = await getRhydbModule();
        const start = Math.max(1, center - width);
        const end = center + width;
        const positions = [];
        for (let p = start; p <= end; p++) positions.push(p);

        const query = readPatternQuery(table, seqColumn, positions);
        const ndjson = module.query(currentHandle, query);
        const rows = ndjson
            .split("\n")
            .filter((line) => line.trim() !== "")
            .map((line) => JSON.parse(line));

        const centerIndex = center - start;
        // Keep only reads that actually cover the centre position (matching "reads
        // around this position"); everything else is background.
        const patterns = [];
        for (const row of rows) {
            const symbols = positions.map((_, i) => String(row[`p${i + 1}`] ?? ""));
            const count = Number(row.n) || 0;
            if (count <= 0 || !isCovered(symbols[centerIndex])) continue;
            patterns.push({ symbols, count });
        }
        patterns.sort((a, b) => b.count - a.count);
        const depth = patterns.reduce((sum, p) => sum + p.count, 0);

        readsState = { positions, centerIndex, patterns, depth, page: 0, table, seqColumn, center };
        renderReads();
    });
}

function renderReads() {
    const out = $("#reads-output");
    out.textContent = "";
    if (!readsState) return;

    const { positions, centerIndex, patterns, depth, center, seqColumn } = readsState;
    const minReads = Math.max(1, Number($("#reads-min").value) || 1);
    const shown = patterns.filter((p) => p.count >= minReads);

    if (depth === 0) {
        out.append(readsNote(`No read covers position ${center} in column "${seqColumn}".`));
        return;
    }
    if (shown.length === 0) {
        out.append(readsNote(`No pattern reaches ${minReads} reads.`));
        return;
    }

    // Consensus = the majority base per column among the shown reads (not a reference).
    const consensus = positions.map((_, col) => {
        const tally = new Map();
        for (const p of shown) {
            const s = p.symbols[col];
            if (!isCovered(s)) continue;
            tally.set(s, (tally.get(s) || 0) + p.count);
        }
        let best = null;
        let bestCount = -1;
        for (const [s, c] of tally) {
            if (c > bestCount) {
                best = s;
                bestCount = c;
            }
        }
        return best;
    });

    const pageCount = Math.max(1, Math.ceil(shown.length / READS_PAGE_SIZE));
    if (readsState.page >= pageCount) readsState.page = pageCount - 1;
    const pageStart = readsState.page * READS_PAGE_SIZE;
    const pageItems = shown.slice(pageStart, pageStart + READS_PAGE_SIZE);

    const scroll = document.createElement("div");
    scroll.className = "reads-scroll";
    const table = document.createElement("table");
    table.className = "reads-grid";

    // Header: unit label, one cell per position (labelled at the centre and every
    // 10th), then the reads/share columns.
    const thead = document.createElement("tr");
    thead.append(readsCell("th", "position", "rowhead"));
    positions.forEach((pos, i) => {
        const cell = readsCell("th", "", `pos poshead${i === centerIndex ? " center" : ""}`);
        if (i === centerIndex || pos % 10 === 0) {
            const label = document.createElement("span");
            label.className = "poslabel";
            label.textContent = String(pos);
            cell.append(label);
        }
        thead.append(cell);
    });
    thead.append(readsCell("th", "reads", "num"));
    thead.append(readsCell("th", "share", "num"));
    const head = document.createElement("thead");
    head.append(thead);
    table.append(head);

    const body = document.createElement("tbody");

    // Consensus row.
    const consensusRow = document.createElement("tr");
    consensusRow.className = "consensus";
    consensusRow.append(readsCell("th", "consensus", "rowhead"));
    consensus.forEach((sym, i) => {
        const cell = readsCell("td", "", `pos${i === centerIndex ? " center" : ""}`);
        if (sym == null) {
            cell.textContent = "·";
            cell.style.color = "#c2c9d0";
        } else {
            cell.textContent = sym;
            cell.style.color = readsColor(sym);
            cell.style.fontWeight = "700";
        }
        consensusRow.append(cell);
    });
    consensusRow.append(readsCell("td", "", "num"));
    consensusRow.append(readsCell("td", "", "num"));
    body.append(consensusRow);

    // One row per read pattern on this page.
    for (const p of pageItems) {
        const row = document.createElement("tr");
        row.append(readsCell("td", "", "rowhead"));
        p.symbols.forEach((sym, i) => {
            const cell = readsCell("td", "", `pos${i === centerIndex ? " center" : ""}`);
            if (!isCovered(sym)) {
                cell.classList.add("hatch");
            } else if (sym === consensus[i]) {
                cell.classList.add("agree");
                cell.textContent = "·";
            } else {
                cell.classList.add("diff");
                cell.textContent = sym;
                cell.style.color = readsColor(sym);
            }
            row.append(cell);
        });
        row.append(readsCell("td", fmtInt(p.count), "num"));
        row.append(readsCell("td", `${((p.count / depth) * 100).toFixed(1)}%`, "num"));
        body.append(row);
    }
    table.append(body);
    scroll.append(table);
    out.append(scroll);

    // Pager + CSV.
    const pager = document.createElement("div");
    pager.className = "reads-pager";
    if (pageCount > 1) {
        const prev = readsButton("‹ Prev", readsState.page === 0, () => {
            readsState.page -= 1;
            renderReads();
        });
        const next = readsButton("Next ›", readsState.page >= pageCount - 1, () => {
            readsState.page += 1;
            renderReads();
        });
        const label = document.createElement("span");
        label.textContent = `Page ${readsState.page + 1} / ${pageCount}`;
        pager.append(prev, label, next);
    }
    const spacer = document.createElement("span");
    spacer.className = "spacer";
    pager.append(spacer, readsButton("Download CSV", false, () => downloadReadsCsv(shown)));
    out.append(pager);

    out.append(
        readsNote(
            `${fmtInt(shown.length)} pattern(s) with ≥ ${minReads} reads cover position ${center}, ` +
                `accounting for ${fmtInt(depth)} reads. The top row is a consensus computed from ` +
                `these reads, not a reference genome. A dot means the read agrees with the ` +
                `consensus; hatched cells are positions a read did not reach.`,
        ),
    );
}

function downloadReadsCsv(patterns) {
    if (!readsState) return;
    const header = [...readsState.positions.map(String), "reads"];
    const lines = [header.join(",")];
    for (const p of patterns) {
        lines.push([...p.symbols, p.count].join(","));
    }
    const blob = new Blob([lines.join("\n")], { type: "text/csv" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `reads-${readsState.seqColumn}-${readsState.center}.csv`;
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 0);
}

function readsCell(tag, text, className) {
    const cell = document.createElement(tag);
    if (className) cell.className = className;
    if (text) cell.textContent = text;
    return cell;
}

function readsButton(text, disabled, onClick) {
    const button = document.createElement("button");
    button.type = "button";
    button.textContent = text;
    button.disabled = disabled;
    button.addEventListener("click", onClick);
    return button;
}

function readsNote(text) {
    const note = document.createElement("p");
    note.className = "reads-info";
    note.textContent = text;
    return note;
}

function fmtInt(value) {
    return Number(value).toLocaleString();
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
