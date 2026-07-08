// Smoke/integration test for the SILO WebAssembly build.
//
// Requires `dist/wasm/silo_wasm.js` / `.wasm` to already be built, e.g. via
// `make wasm`. `make wasm-test` builds it first.

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import test from "node:test";

import createSiloModule from "../../dist/wasm/silo_wasm.js";

const here = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(here, "..", "..");
const fixtureDir = join(repoRoot, "testBaseData", "unitTestDummyDataset");

const EXPECTED_SEQUENCE_COUNT = 5;

function mkdirp(module, path) {
    let current = "";
    for (const part of path.split("/").filter(Boolean)) {
        current += `/${part}`;
        if (!module.FS.analyzePath(current).exists) {
            module.FS.mkdir(current);
        }
    }
}

function readDirectoryEntries(module, path) {
    return module.FS.readdir(path).filter((entry) => entry !== "." && entry !== "..");
}

// Writes the unitTestDummyDataset fixture into the virtual filesystem and
// returns the path to a preprocessing config pointing at it. The config
// overrides `inputDirectory` to the in-memory location, since the checked-in
// preprocessing_config.yaml uses a path relative to the repository root that
// only makes sense for the native build.
function writeFixture(module, inputDir) {
    mkdirp(module, inputDir);

    for (const filename of [
        "input.ndjson",
        "database_config.yaml",
        "reference_genomes.json",
        "test_lineage_definition.yaml",
        "phylogenetic_tree.nwk",
    ]) {
        module.FS.writeFile(
            `${inputDir}/${filename}`,
            readFileSync(join(fixtureDir, filename), "utf8")
        );
    }

    const preprocessingConfig = `
inputDirectory: "."
outputDirectory: "./output/"
ndjsonInputFilename: "input.ndjson"
lineageDefinitionFilenames:
  - "test_lineage_definition.yaml"
phyloTreeFilename: "phylogenetic_tree.nwk"
referenceGenomeFilename: "reference_genomes.json"
`;
    module.FS.writeFile(`${inputDir}/preprocessing_config.yaml`, preprocessingConfig);
}

test("preprocess, query, save, and load a database end-to-end", async () => {
    const module = await createSiloModule();

    writeFixture(module, "/input");
    module.FS.chdir("/input");

    const handle = module.preprocess("preprocessing_config.yaml");
    assert.notEqual(handle, undefined);

    const info = JSON.parse(module.info(handle));
    assert.equal(info.sequenceCount, EXPECTED_SEQUENCE_COUNT);

    const ndjson = module.query(handle, "default.groupBy({count:=count()})");
    const rows = ndjson
        .trim()
        .split("\n")
        .filter((line) => line.length > 0)
        .map((line) => JSON.parse(line));
    assert.equal(rows.length, 1);
    assert.equal(rows[0].count, EXPECTED_SEQUENCE_COUNT);

    mkdirp(module, "/output");
    module.save(handle, "/output");
    // save() creates a versioned data directory under /output (e.g. /output/<version>/),
    // matching the native on-disk layout that load() also expects.
    const savedEntries = readDirectoryEntries(module, "/output");
    assert.ok(savedEntries.length > 0, "save() should create a versioned state directory");

    const loadedHandle = module.load("/output");
    const loadedInfo = JSON.parse(module.info(loadedHandle));
    assert.equal(loadedInfo.sequenceCount, EXPECTED_SEQUENCE_COUNT);

    module.dispose(handle);
    module.dispose(loadedHandle);
});
