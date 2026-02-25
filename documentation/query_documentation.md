# SILO Query Documentation

All queries are sent to the `/query` endpoint as a plain-text request body containing a [SaneQL](https://www.cidrdb.org/cidr2024/papers/p48-neumann.pdf) expression.

The response is NDJSON by default (`application/x-ndjson`), or Apache Arrow IPC if the client sends `Accept: application/vnd.apache.arrow.stream`.

## Query Structure

A query is a **pipeline** of operations chained with `.method()` syntax, starting from a table name:

```
tableName
  .operation1(...)
  .operation2(...)
```

The table name is `default` for now.

Simple example — count all sequences from Switzerland:

```
default
  .filter(country = 'Switzerland')
  .groupBy({count:=count()})
```

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

Keeps only rows where the boolean predicate is true.

```
default.filter(country = 'USA' && age > 30)
```

### `groupBy(aggregates [, columns])`

Aggregates rows, producing counts or other aggregate values. `aggregates` is a record literal; `columns` is an optional set of column names to group by.

Currently supported aggregate function: `count()`.

```
default.groupBy({count:=count()})
default.groupBy({count:=count()}, {pango_lineage})
default.groupBy({count:=count()}, {country, pango_lineage})
```

### `project(fields)`

Returns only the specified columns. `fields` is a set of column names (or a single name without braces).

```
default.project({primary_key, country, date})
default.project(division)
```

Sequence data columns use the naming convention `<sequenceName>` for aligned sequences and `unaligned_<sequenceName>` for unaligned sequences.

### `orderBy(fields)`

Sorts results. Each field is either a bare name (ascending) or a `asc(name)` / `desc(name)` call.

```
default.orderBy({primary_key})
default.orderBy({count.desc(), pango_lineage})
default.orderBy({asc(date), desc(age)})
```

### `limit(count)`

Returns at most `count` rows. Must be a positive integer.

```
default.limit(100)
```

### `offset(count)`

Skips the first `count` rows.

```
default.orderBy({primary_key}).offset(10).limit(10)
```

### `randomize([seed:=n])`

Returns rows in random order. An optional integer seed makes the result reproducible.

```
default.randomize()
default.randomize(seed:=42)
```

### `mutations(minProportion:=p [, sequenceNames:={...}] [, fields:={...}])`

Returns nucleotide mutation statistics for the filtered rows. `minProportion` (0.0–1.0) is the minimum frequency threshold.

Output columns: `mutation`, `mutationFrom`, `mutationTo`, `position`, `sequenceName`, `coverage`, `proportion`, `count`.

```
default.filter(pango_lineage = 'B.1.1.7').mutations(minProportion:=0.05)
default.mutations(minProportion:=0.9, sequenceNames:={main, S})
```

This is only a valid operation on a table or direct filters of a table.

### `aminoAcidMutations(minProportion:=p [, sequenceNames:={...}] [, fields:={...}])`

Same as `mutations` but for amino acid sequences.

This is only a valid operation on a table or direct filters of a table.

```
default.aminoAcidMutations(minProportion:=0.3, sequenceNames:={S})
```

### `insertions([sequenceNames:={...}])`

Returns nucleotide insertions aggregated by insertion value. Output columns: `insertion`, `insertedSymbols`, `position`, `sequenceName`, `count`.

```
default.insertions()
default.insertions(sequenceNames:={main})
```

This is only a valid operation on a table or direct filters of a table.

### `aminoAcidInsertions([sequenceNames:={...}])`

Same as `insertions` but for amino acid sequences.

```
default.aminoAcidInsertions()
```

This is only a valid operation on a table or direct filters of a table.

### `mostRecentCommonAncestor(column [, printNodesNotInTree:=bool])`

Finds the most recent common ancestor in a phylogenetic tree column for the filtered sequences.

```
default.filter(country = 'Germany').mostRecentCommonAncestor('usherTree')
```

This is only a valid operation on a table or direct filters of a table.

### `phyloSubtree(column [, printNodesNotInTree:=bool] [, contractUnaryNodes:=bool])`

Returns the phylogenetic subtree for the filtered sequences.

```
default.filter(pango_lineage = 'B.1.1.7').phyloSubtree('usherTree')
```

This is only a valid operation on a table or direct filters of a table.

---

## Scalar Functions

These are used inside `.filter(...)` predicates. For now, only boolean scalar functions exist.

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
