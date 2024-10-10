import * as fs from "fs";
import csvParser from "csv-parser";
import * as path from "path";
import * as duckdb from "duckdb";
import * as yaml from "js-yaml";
import { tmpdir } from "os";
import { Command } from "commander";
import { RowData } from "duckdb";
import * as arrow from "apache-arrow";

const program = new Command();

program
  .argument("<aliasKey>", "Path to the alias key in JSON format")
  .argument("<lineageFile>", "Path to the input file containing all lineages")
  .option(
    "--preserve-tmp-dir",
    "Preserve the temporary directory to keep the intermediate duckdb tables",
  )
  .option("--verbose", "Verbose logging")
  .parse(process.argv);

const options = program.opts();
const aliasKeyPath = program.args[0];
const lineageFilePath = program.args[1];

let aliasTable: any[] = [];
let aliasDict: { [key: string]: string[] } = {};

// Read and process the alias key file
const aliasFile = JSON.parse(fs.readFileSync(aliasKeyPath, "utf-8"));

for (const [key, value] of Object.entries(aliasFile)) {
  if (typeof value === "string") {
    if (value && value.length > 0) {
      aliasTable.push({ name: key, alias: value });
      aliasDict[key] = [value];
    } else {
      aliasTable.push({ name: key, alias: key });
      aliasDict[key] = [key];
    }
  } else if (Array.isArray(value)) {
    aliasDict[key] = value as string[];
  }
}

function unaliasLineage(lineage: string): string {
  const parts = lineage.split(".");
  const firstPart = parts[0];
  if (aliasDict[firstPart] && aliasDict[firstPart].length === 1) {
    parts[0] = aliasDict[firstPart][0];
    return parts.join(".");
  } else {
    return lineage;
  }
}

function findImmediateParent(lineage: string): string | null {
  if (lineage.includes(".")) {
    return lineage.substring(0, lineage.lastIndexOf("."));
  }
  return null;
}

let lineageTable: any[] = [];
let allUnaliased = new Set<string>();

// Read and process the lineage CSV file
fs.createReadStream(lineageFilePath)
  .pipe(csvParser())
  .on("data", (row: any) => {
    const lineage = row[Object.keys(row)[0]];
    const unaliased = unaliasLineage(lineage);
    allUnaliased.add(unaliased);
    const parentLineageUnaliased = findImmediateParent(unaliased);
    lineageTable.push({
      lineage,
      unaliased,
      parentLineageUnaliased,
    });
  })
  .on("end", () => {
    // Fill "gaps" in lineage system: e.g. generate BA if it is missing and BA.something is present
    let idx = 0;
    while (idx < lineageTable.length) {
      const unaliased = lineageTable[idx].parentLineageUnaliased;
      if (unaliased && !allUnaliased.has(unaliased)) {
        allUnaliased.add(unaliased);
        const parentLineageUnaliased = findImmediateParent(unaliased);
        lineageTable.push({
          lineage: unaliased,
          unaliased,
          parentLineageUnaliased,
        });
      }
      idx++;
    }

    const tempDirPrefix = path.join(tmpdir(), "silo-lineage-definitions-");
    const tempDir = fs.mkdtempSync(tempDirPrefix);
    if (options.verbose) {
      console.error(`Temporary directory: ${tempDir}`);
    }

    const dbPath = path.join(tempDir, "lineage_transform.duckdb");
    const db = new duckdb.Database(dbPath);
    const con = db.connect();

    db.exec(`INSTALL arrow; LOAD arrow;`, (err) => {
      if (err) {
        console.warn(err);
        return;
      }

      const arrowTable2 = arrow.tableFromJSON(lineageTable);
      db.register_buffer(
        "lineageTable",
        [arrow.tableToIPC(arrowTable2)],
        true,
        (err, res) => {
          if (err) {
            console.warn(err);
            return;
          }

          let lineageDict: any = {};
          con.each(
            "SELECT lineage FROM lineages ORDER BY lineage",
            [],
            (err, row: RowData) => {
              console.log(row);
              lineageDict[row.lineage] = {};
            },
          );
        },
      );
    });
  });
