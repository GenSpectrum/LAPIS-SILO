# Python Bindings (`silodb`)

`silodb` is a Python package that embeds SILO in-process, exposing the `Database` class. It lets you build or load a SILO database, run [SaneQL](query_documentation.md) queries, edit scalar values, and persist state — all without running the HTTP API server.

Queries return [Apache Arrow](https://arrow.apache.org/) tables (`pyarrow`), and filter results are returned as [Roaring bitmaps](https://roaringbitmap.org/) (`pyroaring`), so results integrate directly with the scientific Python stack.

## Installation

The package is built from this repository with a C++ toolchain (the same one used to build SILO) and Cython. From the repository root:

```bash
make python-tests   # builds silodb into a virtualenv and runs the test suite
```

or build a wheel directly:

```bash
uv build --wheel
```

The bindings depend on `pyarrow` and `pyroaring`, which are installed automatically as dependencies.

## Quick Start

```python
from silodb import Database

# Load a preprocessed database state from a silo-directory
db = Database("path/to/silo-directory")

# Run a SaneQL query; results come back as a pyarrow.Table
table = db.query("default.filter(region = 'Europe').project({primaryKey, age})")
print(table.to_pydict())

# Overwrite a scalar column for the matching rows
db.update_column("default", "age", "0", filter_expression="age = 4")
```

## The `Database` Class

`from silodb import Database`

### Constructing and loading

**`Database(file_name=None)`**
Creates an empty database, or loads an existing one when `file_name` is given.

`file_name` is a **silo-directory**: the directory that holds one or more versioned database states (see [Data Directories](data_directories.md)). The most recent compatible state is loaded automatically.

```python
empty = Database()                       # in-memory, no tables
loaded = Database("path/to/silo-dir")    # loads the most recent state
```

Raises `FileNotFoundError` if the path does not exist, and `RuntimeError` if no valid state can be loaded (for example, an incompatible serialization version).

`Database` is also a context manager:

```python
with Database("path/to/silo-dir") as db:
    result = db.query("default")
```

### Building a database in memory

Two helpers create a table with a primary key, a sequence column, and optional **string** columns. Scalar (int/float/date/bool) columns cannot be created through these helpers; they come from a preprocessed database that you load.

**`create_nucleotide_sequence_table(table_name, primary_key_name, sequence_name, reference_sequence, extra_columns=None)`**
**`create_gene_table(table_name, primary_key_name, gene_name, reference_sequence, extra_columns=None)`**

```python
db = Database()
db.create_nucleotide_sequence_table(
    table_name="sequences",
    primary_key_name="primary_key",
    sequence_name="main",
    reference_sequence="ACGT...",
    extra_columns=["country", "lineage"],   # string columns
)
```

Data is then appended in [NDJSON format](input_format.md):

**`append_data_from_file(table_name, file_name)`** — append records from an NDJSON file.
**`append_data_from_string(table_name, json_string)`** — append records from an NDJSON string.

```python
db.append_data_from_file("sequences", "input.ndjson")
db.append_data_from_string(
    "sequences",
    '{"primary_key": "s1", "main": {"sequence": "ACGT", "insertions": []}, "country": "CH"}',
)
```

### Querying

**`query(query_string)`** → `pyarrow.Table`
Executes a [SaneQL](query_documentation.md) query. The leading identifier is the table name.

```python
result = db.query("default.filter(age >= 18).project({primaryKey, age, country})")
data = result.to_pydict()      # dict of columns
df = result.to_pandas()        # pandas DataFrame
```

**`get_filtered_bitmap(table_name, filter_expression="")`** → `pyroaring.BitMap`
Returns the row indices matching a SaneQL filter expression. An empty or `None` filter defaults to `true` (all rows). Useful for cheap counting and set operations.

```python
matching = db.get_filtered_bitmap("default", "region = 'Europe'")
print(len(matching))                       # number of matching rows
```

### Inspecting the database

**`get_tables()`** → `pyarrow.Table` with a single `table_name` column listing all tables.
**`get_nucleotide_reference_sequence(table_name, sequence_name)`** → `str`.
**`get_amino_acid_reference_sequence(table_name, sequence_name)`** → `str`.
**`print_all_data(table_name)`** — prints all rows of a table to stdout (debugging aid).

### Updating scalar columns

**`update_column(table_name, column_name, value, filter_expression="")`**

Assigns a single scalar `value` to `column_name` for every row matched by `filter_expression` (a SaneQL filter, like `get_filtered_bitmap`). An empty or `None` filter defaults to `true`, updating all rows.

`value` is a **SaneQL literal** — parsed by the same parser as queries — and must match the column's type:

| Column type | `value` examples                     |
| ----------- | ------------------------------------ |
| int         | `"3"`, `"-1"`                         |
| float       | `"3.14"`, `"0"`                       |
| bool        | `"true"`, `"false"`                  |
| date        | `"'2021-03-15'::date"`               |
| any of them | `"null"` &nbsp;(clears the rows)     |

Only scalar value columns (int, float, date, bool) can be updated. String and sequence columns are rejected.

```python
db = Database("path/to/silo-dir")

# Set every row's age to 0
db.update_column("default", "age", "0")

# Set age to 100 only for rows currently equal to 4
db.update_column("default", "age", "100", filter_expression="age = 4")

# Assign a date literal
db.update_column("default", "date", "'2000-01-01'::date")

# Clear a boolean column to null for a subset of rows
db.update_column("default", "test_boolean_column", "null", filter_expression="region = 'Asia'")
```

The update mutates only the **in-memory** database. On-disk state is left untouched until you call `save_checkpoint`. Because the update bumps the data version, a subsequently saved checkpoint is written as a new versioned state.

**Not thread-safe.** `update_column` mutates the in-memory database in place with no internal locking.

### Persisting state

**`save_checkpoint(save_directory)`**
Writes the current database state into `save_directory` as a new versioned state, which can later be reloaded with `Database(save_directory)`.

```python
db.update_column("default", "age", "0")
db.save_checkpoint("out/silo-dir")
reloaded = Database("out/silo-dir")
```

## Error Handling

The bindings translate C++ exceptions into Python ones:

- **`ValueError`** — invalid query or update: an unknown column, an unsupported column type, a literal that does not match the column type, or a malformed SaneQL expression. Empty required arguments (such as `table_name` or `column_name`) also raise `ValueError`.
- **`FileNotFoundError`** — a path passed to the loader or `append_data_from_file` does not exist.
- **`RuntimeError`** — other failures (for example, a failed load or save).

```python
try:
    db.update_column("default", "country", "'x'")   # country is a string column
except ValueError as e:
    print(e)   # "Updating columns of type '...' is not supported; only INT32, FLOAT, DATE32 and BOOL ..."
```

## Complete Example

```python
from silodb import Database

with Database("path/to/silo-dir") as db:
    # Count the rows we are about to change
    before = len(db.get_filtered_bitmap("default", "age = 4"))

    # Reset those rows' age to null
    db.update_column("default", "age", "null", filter_expression="age = 4")

    # Verify via a query
    remaining = len(db.get_filtered_bitmap("default", "age = 4"))
    cleared = len(db.get_filtered_bitmap("default", "age = null"))
    assert remaining == 0
    assert cleared >= before

    # Persist the edited database as a new state
    db.save_checkpoint("out/silo-dir")
```
