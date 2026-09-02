# Incremental Preprocessing with `rhydb append`

The `rhydb append` command adds new records to an existing RhyDB database without requiring a full preprocessing run. It reads data in NDJSON format, appends it to a previously built database state, and writes the resulting updated state to the data directory.

## Overview

A typical RhyDB workflow begins with `rhydb preprocessing`, which builds a database from scratch. As new data becomes available, `rhydb append` can be used to incorporate it incrementally. The command loads an existing database state, parses the new records from an NDJSON source, validates and inserts them, and then persists the updated database as a new versioned state in the data directory.

The append operation is atomic in the sense that the new state is only written after all records have been successfully inserted and validated. If any record fails validation (for example, due to a schema mismatch or a duplicate primary key), the operation aborts and the existing state remains untouched.

## Usage

```
rhydb append [options...]
```

### Options

All options can also be provided via environment variables or a YAML configuration file. Options override environment variables, which override YAML file entries.

**`--data-directory <path>`** (default: `.`)
The path to a data directory. This directory is used both as the source of existing database states (unless `--data-source` is specified) and as the destination for the newly produced state.
Environment variable: `RHYDB_DATA_DIRECTORY`. YAML key: `dataDirectory`.

**`--append-file <path>`** (optional)
The path to an NDJSON file containing the records to append. Compressed files (`.zst` and `.xz`) are detected and decompressed transparently. If this option is omitted, data is read from stdin instead.
Environment variable: `RHYDB_APPEND_FILE`. YAML key: `appendFile`.

**`--data-source <path>`** (optional)
A directory containing a valid RhyDB database state to use as the base for appending. If omitted, `rhydb append` automatically selects the most recent compatible state from the data directory. An error is raised if no valid state can be found.
Environment variable: `RHYDB_DATA_SOURCE`. YAML key: `dataSource`.

## Input Format

The input must be in Newline-Delimited JSON (NDJSON) format: one JSON object per line. Each object represents a single record and must contain all columns defined in the database schema. Unknown fields are ignored with a warning; missing required fields cause an error.

The field order is determined from the first line and reused for all subsequent lines. For best performance, all lines should use the same field ordering. If a line deviates, RhyDB falls back to an unordered lookup and logs a warning.

See `input_format.md` for further details.

## How It Works

When `rhydb append` is invoked, the following steps are performed:

1. The data directory is opened, and the base database state is determined -- either from the explicitly provided `--data-source` or by scanning the data directory for the most recent compatible state.

2. The existing database is loaded from disk into memory.

3. The NDJSON input is opened, either from the file specified by `--append-file` or from stdin.

4. The first line of the input is parsed to determine the field order and to validate that all required schema columns are present.

5. Each subsequent line is parsed and its values are inserted into the database. Progress is logged every 10,000 records.

6. A new data version is assigned, and the updated database state is saved as a new timestamped subdirectory within the data directory.

## Examples

Append data from a file to the database in the current directory:

```
rhydb append --append-file new_sequences.ndjson
```

Append compressed data to a specific data directory:

```
rhydb append --data-directory /data/silo --append-file new_sequences.ndjson.zst
```

Pipe data from another process into append:

```
generate_data | rhydb append --data-directory /data/silo
```

Append to a specific base state rather than the most recent one:

```
rhydb append --data-directory /data/silo --data-source /data/silo/20240101T120000
```
