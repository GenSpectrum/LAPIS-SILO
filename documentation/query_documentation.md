# SILO Query Documentation

All queries are sent to the `/query` endpoint as a plain-text request body containing a [SaneQL](https://www.cidrdb.org/cidr2024/papers/p48-neumann.pdf) expression.

The response is NDJSON by default (`application/x-ndjson`), or Apache Arrow IPC if the client sends `Accept: application/vnd.apache.arrow.stream`.

## Query Structure

A query is a **pipeline** of operators chained with `.method()` syntax, starting from a table name:

```
tableName
  .operator1(...)
  .operator2(...)
```

The table name is `default` for now.

### Tabular data model

Every operator takes a table as input and produces a table as output. Internally these tables are Apache Arrow record batches; externally they are streamed as NDJSON or Arrow IPC. The **response schema** — which fields are returned and their types — is always the output schema of the **last operator** in the pipeline.

Simple example — count all sequences from Switzerland:

```
default
  .filter(country = 'Switzerland')
  .groupBy({count:=count()})
```

`filter` is schema-preserving; `groupBy` is last and schema-defining, so the response is `{"count": <integer>}`.

## Language Basics

### Literals

| Type    | Syntax                                      | Example                        |
|---------|---------------------------------------------|--------------------------------|
| String  | single-quoted                               | `'Switzerland'`                |
| Integer | bare number                                 | `42`                           |
| Float   | decimal                                     | `3.14`                         |
| Boolean | `true` / `false`                            | `true`                         |
| Null    | `null`                                      | `null`                         |
| Date    | `'YYYY-MM-DD'::date`                        | `'2021-03-15'::date`           |
| Set     | `{elem1, elem2, ...}`                       | `{'A', 'B', 'C'}`              |
| Record  | `{field1 := value1, field2 := value2, ...}` | `{x := 'A', y := 'B', z := 3}` |

### Boolean Operators

| Operator | Meaning |
|----------|---------|
| `&&` | logical AND |
| `\|\|` | logical OR |
| `!expr` | logical NOT |
| `(expr)` | grouping |

### Comparison Operators

WARNING: Not all operators have been implemented for all types. TODO()

`=`, `<>`, `<`, `<=`, `>`, `>=`

The left-hand side must be a column identifier. Examples:
```
country = 'Germany'
age > 30
date <= '2021-12-31'::date
qc_value <> null
```

### Method Call Syntax

Any function `f(table, arg1, arg2)` can be written as `table.f(arg1, arg2)`. Named arguments use `:=`:

```
pango_lineage.lineage('B.1.1.7', includeSublineages:=true)
```

After the first named argument is given, no more positional arguments are accepted.

---

## Pipeline Operations

### `filter(predicate)`

Keeps only rows where the boolean predicate is true. Passes all input columns through unchanged.

```
default.filter(country = 'USA' && age > 30)
```

### `groupBy(aggregates [, columns])`

Aggregates rows, producing counts or other aggregate values. `aggregates` is a record literal; `columns` is an optional set of column names to group by.

Currently supported aggregate function: `count()`.

```
default.groupBy(aggregates:={count:=count()})
default.groupBy(aggregates:={count:=count()}, columns:={pango_lineage})
default.groupBy({count:=count()}, {country, pango_lineage})
```

**Output:** one row per group, containing the named aggregation fields and the groupBy columns. Rows where a groupBy column is null form their own group with a null value for that column.

```json
{"count": 48, "pango_lineage": "B.1.1.7"}
{"count": 1,  "pango_lineage": null}
```

### `project(fields)`

Returns only the specified columns. `fields` is a set of column names (or a single name without braces).

```
default.project({primary_key, country, date, pango_lineage, qc_value})
default.project(division)
```

Sequence data columns use the naming convention `<sequenceName>` for aligned sequences and `unaligned_<sequenceName>` for unaligned sequences.

**Output:** one row per input row containing only the projected columns. Values reflect the column type: strings, integers, floats, booleans, dates (`"YYYY-MM-DD"` string in ndjson output), or `null`.

```json
{"primary_key": "key_31", "country": "Switzerland", "date": "2021-03-21", "pango_lineage": "B.1.1.7", "qc_value": 0.96}
```

### `map(expressions)`

Adds columns to the table. `expressions` is a record of `name := value` assignments. Each value may be a literal (integers, floats, single-quoted strings, or booleans), a field reference, or a call to a non-boolean [scalar function](#scalar-functions) such as [`at`](#atcolumn-position).

```
default.map({x := 3, label := 'cohort A', active := true, copy := country})
default.map({second_char := primary_key.at(2)})
```

All input columns are passed through unchanged and the new columns are appended. An assignment whose name matches an existing column replaces that column in place.

**Output:** one row per input row containing all input columns plus the assigned columns. Integer literals become `INT32` (or `INT64` when out of `INT32` range), floats become `FLOAT`, single-quoted literals become `STRING`, and `true`/`false` become `BOOL`.

```json
{"primary_key": "key_31", "x": 3, "label": "cohort A", "active": true, "copy": "Switzerland"}
```

### `orderBy(fields)`

Sorts results. Each field is either a bare name (ascending) or a `asc(name)` / `desc(name)` call. Passes all input columns through unchanged.

```
default.orderBy({primary_key})
default.orderBy({count.desc(), pango_lineage})
default.orderBy({asc(date), desc(age)})
```

### `limit(count)`

Returns at most `count` rows. Must be a positive integer. Passes all input columns through unchanged.

```
default.limit(100)
```

### `offset(count)`

Skips the first `count` rows. Passes all input columns through unchanged.

```
default.orderBy({primary_key}).offset(10).limit(10)
```

### `randomize([seed:=n])`

Returns rows in random order. An optional integer seed makes the result reproducible. Passes all input columns through unchanged.

```
default.randomize()
default.randomize(seed:=42)
```

### `mutations(minProportion:=p [, sequenceNames:={...}] [, fields:={...}])`

Returns nucleotide mutation statistics for the filtered rows. `minProportion` (0.0–1.0) is the minimum frequency threshold. Only valid on a table or direct filters of a table.

```
default.filter(pango_lineage = 'B.1.1.7').mutations(minProportion:=0.05)
default.mutations(minProportion:=0.9, sequenceNames:={main, S})
```

**Output:** one row per mutation meeting the threshold. The field set can be narrowed via `fields:={...}`.

| Field | Type | Description |
|-------|------|-------------|
| `mutation` | string | Substitution formatted as `<from><position><to>`, e.g. `A23403G`, `E156-` |
| `mutationFrom` | string | Reference symbol at this position |
| `mutationTo` | string | Observed symbol (`-` for a deletion) |
| `position` | integer | 1-based position in the sequence |
| `sequenceName` | string | Name of the sequence, e.g. `main` or `S` |
| `proportion` | float | Fraction of covered sequences that carry this mutation |
| `coverage` | integer | Number of sequences with a non-N symbol at this position |
| `count` | integer | Number of sequences carrying this mutation |

```json
{"mutation": "N501Y", "mutationFrom": "N", "mutationTo": "Y", "position": 501, "sequenceName": "S", "proportion": 0.44086021505376344, "coverage": 93, "count": 41}
```

### `aminoAcidMutations(minProportion:=p [, sequenceNames:={...}] [, fields:={...}])`

Same as `mutations` but for amino acid sequences. Output schema is identical. Only valid on a table or direct filters of a table.

```
default.aminoAcidMutations(minProportion:=0.3, sequenceNames:={S})
```

### `insertions([sequenceNames:={...}])`

Returns nucleotide insertions aggregated by insertion value. Only valid on a table or direct filters of a table.

```
default.insertions()
default.insertions(sequenceNames:={main})
```

**Output:** one row per unique insertion observed across the filtered sequences.

| Field | Type | Description |
|-------|------|-------------|
| `insertion` | string | Formatted as `ins_<position>:<symbols>`, e.g. `ins_22204:CAGAA` |
| `insertedSymbols` | string | The inserted nucleotide sequence |
| `position` | integer | 1-based position after which the insertion occurs (0 = before position 1) |
| `sequenceName` | string | Name of the sequence |
| `count` | integer | Number of sequences carrying this exact insertion |

```json
{"insertion": "ins_22204:CAGAA", "insertedSymbols": "CAGAA", "position": 22204, "sequenceName": "main", "count": 1}
```

### `aminoAcidInsertions([sequenceNames:={...}])`

Same as `insertions` but for amino acid sequences. Output schema is identical. Only valid on a table or direct filters of a table.

```
default.aminoAcidInsertions()
```

### `mostRecentCommonAncestor(column [, printNodesNotInTree:=bool])`

Finds the most recent common ancestor in a phylogenetic tree column for the filtered sequences. Only valid on a table or direct filters of a table.

```
default.filter(country = 'Germany').mostRecentCommonAncestor('usherTree')
```

**Output:** a single row.

| Field | Type | Description |
|-------|------|-------------|
| `mrcaNode` | string | Node ID of the most recent common ancestor |
| `mrcaParent` | string | Parent node ID of the MRCA |
| `mrcaDepth` | integer | Depth of the MRCA in the tree (root = 0) |
| `missingNodeCount` | integer | Number of filtered sequences not found in the tree |
| `missingFromTree` | string | Comma-separated primary keys of sequences not in the tree. Only present when `printNodesNotInTree:=true` |

```json
{"mrcaNode": "NODE_0000072", "mrcaParent": "NODE_0000070", "mrcaDepth": 23, "missingNodeCount": 0}
```

### `phyloSubtree(column [, printNodesNotInTree:=bool] [, contractUnaryNodes:=bool])`

Returns the phylogenetic subtree for the filtered sequences. Only valid on a table or direct filters of a table.

```
default.filter(pango_lineage = 'B.1.1.7').phyloSubtree('usherTree')
```

**Output:** a single row.

| Field | Type | Description |
|-------|------|-------------|
| `subtreeNewick` | string | Newick-format string of the subtree spanning the filtered sequences |
| `missingNodeCount` | integer | Number of filtered sequences not found in the tree |
| `missingFromTree` | string | Comma-separated primary keys of sequences not in the tree. Only present when `printNodesNotInTree:=true` |

```json
{"subtreeNewick": "((key_83:0.00027051)NODE_0000077:3.291e-05,(...)NODE_0000079:1e-06)NODE_0000076;", "missingNodeCount": 0}
```

### `unionAll(left, right)`

Concatenates the output of two pipelines. `unionAll` can be called as a standalone function or with piped syntax:

Both inputs must have the same schema (same column names, types, and order).

All rows from both inputs are included — duplicates are preserved (UNION ALL, not UNION).

```
unionAll(
  default.filter(division='Aargau').project({division}),
  default.filter(division='Bern').project({division})
)
```

Or equivalently using piped syntax:

```
default.filter(division='Aargau').project({division})
  .unionAll(default.filter(division='Bern').project({division}))
```

Named arguments are also supported:

```
unionAll(left := <pipeline1>, right := <pipeline2>)
```

The result can be piped into downstream operators:

```
unionAll(
  default.filter(division='Aargau').project({division}),
  default.filter(division='Bern').project({division})
).groupBy({count:=count()}, {division})
 .orderBy({asc(division)})
```

`unionAll` calls can be nested:

```
unionAll(
  unionAll(pipelineA, pipelineB),
  unionAll(pipelineC, pipelineD)
)
```

**Restrictions:**

- `mutations()`, `aminoAcidMutations()`, `insertions()`, and similar operators that require a table scan cannot be applied to the result of a `unionAll`. They can however be used inside each child.

Filters above a `unionAll` are automatically pushed into both children:

```
unionAll(
  default.project({primaryKey, country}),
  default.project({primaryKey, country})
).filter(country='CH')
```

is equivalent to:

```
unionAll(
  default.filter(country='CH').project({primaryKey, country}),
  default.filter(country='CH').project({primaryKey, country})
)
```

**Output:** all rows from both inputs. The order of rows is not guaranteed.

### `schema()`

Describes the output schema of whatever it is applied to — a table or the result of any pipeline.
It does not read or return any data; it only reports the fields that the input would produce.

```
default.schema()
default.filter(country='CH').groupBy({count:=count()}, {age}).schema()
default.mutations(minProportion:=0.1).schema()
```

**Output:** one row per field of the described result, with two columns:

| Field       | Type   | Description                                  |
|-------------|--------|----------------------------------------------|
| `fieldName` | string | Name of the field                            |
| `type`      | string | Type of the field (e.g. `STRING`, `INT32`, `INT64`, `DATE32`, `BOOL`, `FLOAT`, `INDEXED_STRING`) |

```json
{"fieldName": "age", "type": "INT32"}
{"fieldName": "count", "type": "INT64"}
```

`schema()` produces an ordinary two-column relation,
so schema-preserving and schema-defining operators (such as `project` and `orderBy`) can be chained after it.

**Limitation:** sequence columns are reported with type `STRING`.
When a sequence column is read into a pipeline it is decompressed to a string before `schema()` observes it,
so nucleotide and amino acid sequences cannot be distinguished from ordinary strings at this point.

**Limitation:** `schema()` is a result-producing source, so `filter(...)` cannot be applied to its output.
Filtering is realized by pushing the predicate down into the underlying data source,
and there is no data source above `schema()` to push into.

---

## Scalar Functions

Most scalar functions are boolean predicates used inside `.filter(...)`. Functions that return a non-boolean value (currently [`at`](#atcolumn-position)) cannot be used as a filter predicate and are instead used as `map()` assignments.

### `at(column, position)`

Extracts the single character of a string `column` at the 1-based `position`, returning it as a string. `position` must be greater than 0. If `position` is past the end of the value, the result is an empty string `""` (a `null` value yields `null`). This is not a boolean predicate, so it cannot be used in `.filter(...)`; use it inside [`map()`](#mapexpressions).

```
default.map({second_char := primary_key.at(2)})
```

### `between(column, from, to)`

True if `column` is between `from` and `to` (inclusive). Use `null` for an open bound. Works for dates, integers, and floats.

WARNING: Currently float is non-inclusive for `to`.

```
date.between('2021-01-01'::date, '2021-12-31'::date)
age.between(18, 65)
qc_value.between(0.9, null)
```

Alternatively, use comparison operators:
```
date >= '2021-01-01'::date && date <= '2021-12-31'::date
```

### `in(column, {values})`

True if the column value is one of the given strings.

```
country.in({'Germany', 'France', 'Italy'})
```

### `isNull(column)`

True if the column value is NULL.

```
isNull(date)
```

### `isNotNull(column)`

True if the column value is not NULL.

```
isNotNull(pango_lineage)
```

### `like(column, pattern)`

True if the column value matches the regular expression `pattern`. Uses [RE2 syntax](https://github.com/google/re2/wiki/Syntax).

```
division.like('Basel.*')
primary_key.like('key_[0-9]+')
```

### `lineage(column, value [, includeSublineages:=bool] [, recombinantFollowingMode:=string])`

True if the lineage column matches `value`. Column must have `generateLineageIndex: true` in the schema.

`includeSublineages` (default `false`) also matches sublineages of `value`. `value` may be `null` to match NULL rows.

`recombinantFollowingMode` controls handling of recombinant lineages when `includeSublineages` is `true`:
- `"doNotFollow"` (default) — only non-recombinant parent-child relationships
- `"alwaysFollow"` — include recombinants with at least one parent in the searched clade
- `"followIfFullyContainedInClade"` — include recombinants only if all parents are in the clade

```
pango_lineage.lineage('B.1.1.7')
pango_lineage.lineage('B.1.1.7', includeSublineages:=true)
pango_lineage.lineage('XBB', includeSublineages:=true, recombinantFollowingMode:='alwaysFollow')
```

### `phyloDescendantOf(column, node)`

True if the phylogenetic tree column value is a descendant of `node`.

```
usherTree.phyloDescendantOf('NODE_0000072')
```

### `nucleotideEquals(position:=n, symbol:=s [, sequenceName:=name])`

True if the nucleotide sequence has symbol `s` at 1-based position `n`. Use `.` as a wildcard symbol (matches the reference). `sequenceName` is required if there is more than one nucleotide sequence.

```
nucleotideEquals(position:=300, symbol:='G')
nucleotideEquals(position:=100, symbol:='A', sequenceName:='main')
```

### `aminoAcidEquals(position:=n, symbol:=s [, sequenceName:=name])`

Same as `nucleotideEquals` but for amino acid sequences.

```
aminoAcidEquals(position:=501, symbol:='Y', sequenceName:='S')
```

### `hasMutation(position:=n [, sequenceName:=name])`

True if the nucleotide sequence has a symbol at position `n` that differs from the reference and is not `N`.

```
hasMutation(position:=23403)
hasMutation(position:=100, sequenceName:='main')
```

### `hasAAMutation(position:=n [, sequenceName:=name])`

Same as `hasMutation` but for amino acid sequences.

```
hasAAMutation(position:=501, sequenceName:='S')
```

### `insertionContains(position:=n, value:=regex [, sequenceName:=name])`

True if the nucleotide sequence has an insertion after 1-based position `n` that matches regex `value`. Position 0 means before the first symbol. The regex may contain valid nucleotide symbols and `.*`.

```
insertionContains(position:=22204, value:='AGT')
insertionContains(position:=100, value:='A.*G', sequenceName:='main')
```

### `aminoAcidInsertionContains(position:=n, value:=regex [, sequenceName:=name])`

Same as `insertionContains` for amino acid sequences. The stop-codon symbol `*` must be escaped as `\\*` in the regex.

```
aminoAcidInsertionContains(position:=214, value:='.*EPE', sequenceName:='S')
```

### `maybe(child)`

Relaxes the child expression: true if the child is possibly true (allowing ambiguous symbols). See [maybe_documentation.md](maybe_documentation.md).

```
maybe(nucleotideEquals(position:=122, symbol:='A'))
```

### `exact(child)`

Tightens the child expression: requires an exact (non-ambiguous) match. See [maybe_documentation.md](maybe_documentation.md).

```
exact(nucleotideEquals(position:=300, symbol:='G'))
```

### `nOf(count, {children} [, matchExactly:=bool])`

True if at least `count` of the child expressions are true. If `matchExactly` is `true`, returns true if exactly `count` child expressions are true.

```
nOf(2, {
  nucleotideEquals(position:=241, symbol:='T'),
  nucleotideEquals(position:=3037, symbol:='T'),
  nucleotideEquals(position:=23403, symbol:='G')
})
```

### `nucleotideMutationProfile(distance:=n, ..., [sequenceName:=name])`

True if a sequence has at most `distance` **conservative differences** from a profile sequence.

A position counts as a difference when the database sequence's symbol is **not** ambiguity-compatible with the profile symbol at that position (e.g. `R` is compatible with `A` because `R` represents A or G; `N` is compatible with any definitive base). Positions where the profile symbol is `N` (missing) are skipped and never counted as differences.

`sequenceName` is optional; when omitted the database's default nucleotide sequence is used.

**Profile input — exactly one of the following named arguments:**

- `querySequence:='<seq>'` — a full sequence string of the same length as the reference. Each character must be a valid nucleotide symbol.
- `sequenceId:='<id>'` — the primary key of a sequence already in the database, used as the profile.
- `mutations:={{position:=n, symbol:='X'}, ...}` — a set of mutations relative to the reference. Positions are 1-based. Positions not listed retain the reference symbol. An empty set `{}` means the profile equals the reference.

```
-- Within 5 mutations of the reference (empty mutation list = reference)
nucleotideMutationProfile(distance:=5, mutations:={})

-- Within 2 mutations of a specific stored sequence
nucleotideMutationProfile(distance:=2, sequenceId:='key_123')

-- Within 3 mutations of a profile defined by two substitutions
nucleotideMutationProfile(distance:=3, mutations:={
  {position:=241, symbol:='T'},
  {position:=23403, symbol:='G'}
})

-- Same but targeting a named sequence in a multi-segment database
nucleotideMutationProfile(distance:=3, sequenceName:='S', mutations:={
  {position:=501, symbol:='Y'}
})
```

### `aminoAcidMutationProfile(distance:=n, ..., [sequenceName:=name])`

Same as `nucleotideMutationProfile` but for amino acid sequences. `symbol` values must be valid amino acid characters.

```
aminoAcidMutationProfile(distance:=2, sequenceName:='S', mutations:={
  {position:=501, symbol:='Y'},
  {position:=452, symbol:='R'}
})
```

---

## Complete Examples

### Count sequences by country, ordered by count

```
default
  .groupBy({count:=count()}, {country})
  .orderBy({count.desc()})
```

### Sequences with a specific mutation, showing details

```
default
  .filter(hasMutation(position:=23403))
  .project({primary_key, country, date, pango_lineage})
  .orderBy({date})
  .limit(100)
```

### Mutations above 5% prevalence in a lineage

```
default
  .filter(pango_lineage.lineage('B.1.1.7', includeSublineages:=true))
  .mutations(minProportion:=0.05, sequenceNames:={main})
```

### Date range filter with null exclusion

```
default
  .filter(date.between('2021-01-01'::date, '2021-06-30'::date))
  .groupBy({count:=count()}, {pango_lineage})
  .orderBy({pango_lineage})
```

### Complex filter combining multiple conditions

```
default
  .filter(
    country = 'Germany'
    && age > 18
    && pango_lineage.lineage('B.1.1.7', includeSublineages:=true)
    && nOf(2, {
         nucleotideEquals(position:=241, symbol:='T'),
         nucleotideEquals(position:=3037, symbol:='T'),
         nucleotideEquals(position:=23403, symbol:='G')
       })
  )
  .groupBy({count:=count()})
```

### Paginated results

```
default
  .orderBy({primary_key})
  .offset(50)
  .limit(25)
  .project({primary_key, country, date})
```

### Amino acid insertions with a filter

```
default
  .filter(aminoAcidInsertionContains(position:=214, value:='.*PE', sequenceName:='S'))
  .aminoAcidInsertions()
  .orderBy({insertedSymbols, position})
```

### Combine two filtered groups with unionAll

```
unionAll(
  default.filter(division='Aargau').project({division}),
  default.filter(division='Bern').project({division})
).groupBy({count:=count()}, {division})
 .orderBy({asc(division)})
```
