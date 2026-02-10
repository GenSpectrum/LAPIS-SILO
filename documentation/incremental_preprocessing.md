# Incremental Preprocessing with `silo append`

The `silo append` command adds new records to an existing SILO database without requiring a full preprocessing run. It reads data in NDJSON format, appends it to a previously built database state, and writes the resulting updated state to the silo-directory.

## Overview

A typical SILO workflow begins with `silo preprocessing`, which builds a database from scratch. As new data becomes available, `silo append` can be used to incorporate it incrementally. The command loads an existing database state, parses the new records from an NDJSON source, validates and inserts them, and then persists the updated database as a new versioned state in the silo-directory.

The append operation is atomic in the sense that the new state is only written after all records have been successfully inserted and validated. If any record fails validation (for example, due to a schema mismatch or a duplicate primary key), the operation aborts and the existing state remains untouched.

## Usage

```
silo append [options...]
```

### Options

All options can also be provided via environment variables or a YAML configuration file. Options override environment variables, which override YAML file entries.

**`--silo-directory <path>`** (default: `.`)
The path to a silo-directory. This directory is used both as the source of existing database states (unless `--silo-data-source` is specified) and as the destination for the newly produced state.
Environment variable: `SILO_SILO_DIRECTORY`. YAML key: `siloDirectory`.

**`--append-file <path>`** (optional)
The path to an NDJSON file containing the records to append. Compressed files (`.zst` and `.xz`) are detected and decompressed transparently. If this option is omitted, data is read from stdin instead.
Environment variable: `SILO_APPEND_FILE`. YAML key: `appendFile`.

**`--silo-data-source <path>`** (optional)
A directory containing a valid silo database state to use as the base for appending. If omitted, `silo append` automatically selects the most recent compatible state from the silo-directory. An error is raised if no valid state can be found.
Environment variable: `SILO_SILO_DATA_SOURCE`. YAML key: `siloDataSource`.

## Input Format

The input must be in Newline-Delimited JSON (NDJSON) format: one JSON object per line. Each object represents a single record and must contain all columns defined in the database schema. Unknown fields are ignored with a warning; missing required fields cause an error.

The field order is determined from the first line and reused for all subsequent lines. For best performance, all lines should use the same field ordering. If a line deviates, SILO falls back to an unordered lookup and logs a warning.

### Metadata columns

Metadata columns are represented as top-level key-value pairs in each JSON object. The value types must match the database schema. Null values are supported for columns that allow them.

```json
{
  ...
  "primaryKey": "seq_001",
  "date": "2021-03-18",
  "country": "Switzerland",
  "age": 42,
  "qc_value": 0.98,
  "test_boolean_column": true
  ...
}
```

### Sequence columns

Sequence columns (nucleotide or amino acid) are represented as nested objects with a `sequence` field and an `insertions` array. The sequence string must conform to the expected alphabet and length. Insertions are encoded as `"position:sequence"` strings. A sequence column may also be `null`.

```json
{
  ...
  "main": {
    "sequence": "ACGTACGT",
    "insertions": ["214:EPE"]
  }
  ...
}
```

## How It Works

When `silo append` is invoked, the following steps are performed:

1. The silo-directory is opened, and the base database state is determined -- either from the explicitly provided `--silo-data-source` or by scanning the silo-directory for the most recent compatible state.

2. The existing database is loaded from disk into memory.

3. The NDJSON input is opened, either from the file specified by `--append-file` or from stdin.

4. The first line of the input is parsed to determine the field order and to validate that all required schema columns are present.

5. Each subsequent line is parsed and its values are inserted into the database. Progress is logged every 10,000 records.

6. After all records have been inserted, the database is finalized and validated. This includes checking for duplicate primary keys across both old and new data.

7. A new data version is assigned, and the updated database state is saved as a new timestamped subdirectory within the silo-directory.

## Validation

SILO validates the input at multiple levels during append. Each NDJSON line must parse as a valid JSON object. The first line must contain all columns defined in the database schema. Column values must match their expected types (string, integer, float, boolean, date, or sequence). Primary keys must be unique across the entire database; duplicates cause the operation to fail. Sequence data is validated for correct alphabet symbols and expected lengths.

When validation fails, the error message includes the offending input line for diagnosis.

## Examples

Append data from a file to the database in the current directory:

```
silo append --append-file new_sequences.ndjson
```

Append compressed data to a specific silo-directory:

```
silo append --silo-directory /data/silo --append-file new_sequences.ndjson.zst
```

Pipe data from another process into append:

```
generate_data | silo append --silo-directory /data/silo
```

Append to a specific base state rather than the most recent one:

```
silo append --silo-directory /data/silo --silo-data-source /data/silo/20240101T120000
```
