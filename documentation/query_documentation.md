# Silo Query Documentation

WARNING: This document is work-in-progress and offers some preliminary documentation

All queries are accepted by the same `/query` endpoint.

Each query consists of a `filter` and an `action`, described separately below.

Example files can be seen in the [repository](https://github.com/GenSpectrum/LAPIS-SILO/tree/main/endToEndTests/test/queries).

## Filter

This is similar to the SQL `WHERE` clause. We directly get an abstract syntax tree of a boolean expression in JSON format.

Any `Filter` is a json object that contains a `type` and a number of additional attribute, depending on the type as listed below. The types can be grouped as follows:

### n-ary 

always have attribute `children: Filter[]`

####  `And`

  This filter is true iff all its children are true.
 
#### `Or`

This filter is true iff at least one of its children is true.

#### `N-Of`: `{"numberOfMatchers": number, "matchExactly": boolean}`

This filter is true iff (at least) `numberOfMatchers` of its children are true.

### 1-ary 
always have the attribute `child: Filter`

#### `Not`

This filter is true iff its child is false.

#### `Maybe`

See [`maybe_documentation.md`](maybe_documentation.md).

#### `Exact`

See [`maybe_documentation.md`](maybe_documentation.md).

### Leaf Nodes

#### `True`

This filter is always true.

#### `False`

This filter is always false.

#### `NucleotideEquals`: `{"position": number, "symbol": string, "sequenceName": string}` 

This filter is true, if the symbol of nucleotide sequence `sequenceName` at position `position` is equal to `symbol`. (If this is in a subtree of an `Exact` or `Maybe` expression, it will instead consider all sets of possible non-ambiguous symbols at this position).
`sequenceName` must refer to a valid nucleotide sequence. `symbol` should contain a single valid sequence symbol, `position` a number between 1 and the sequence's length. 

#### `DateBetween`: `{"column": string, "from": string | null, "to": string | null}`

This filter is true if the date in column `column` is between `from` and `to`. 
If from or to is `null` it will not constrain the dates in that regard. `null` dates will always be excluded.

The `from` and `to` bounds are inclusive.

#### `HasNucleotideMutation`: `{"position": number, "sequenceName": string}`

This filter is true if the sequence `sequenceName` has a symbol at position `position` that is not equal to the reference and not `N`. 

#### `AminoAcidEquals`

See `NucleotideEquals`.

#### `HasAminoAcidMutation`

See `HasNucleotideMutation`.

#### `Lineage`: `{"column": string, "value": string, "includeSublineages": boolean}`

This filter is true if the lineage in column `column` is equal to or an alias of `value`. 
If `includeSublineages` is set, it will also be true, if it is equal to or an alias of a sublineage of `value`.
`value` must be a valid lineage (that is contained in the lineage definitions of this column).

`column` must be a string field with `generateLineageIndex: true`.

#### `InsertionContains`: `{"sequenceName": string, "position": string, "value":string}`

This filter is true if the sequence `sequenceName` has an insertion at `position` that has a full match with the regex `value`.
The regex may only contain valid nucleotide symbols and `.*`. 

#### `AminoAcidInsertionContains`

See `InsertionContains`. Note that the stop-codon symbol `*` must be escaped with a `\\`. 

#### `StringSearch`: `{"column": string, "searchExpression": string}`

This filter is true if the string in column `column` matches the regular expression `searchExpression`.

See [google/re2](https://github.com/google/re2/wiki/Syntax) for regular expression syntax.

#### `StringEquals`: `{"column": string, "value":string}`

This filter is true if the string in column `column` is equal to `value`. 

#### `BooleanEquals`: `{"column": string, "value":string}`

This filter is true if the boolean in column `column` is equal to `value`.

#### `IntEquals`: `{"column": string, "value":string}`

This filter is true if the integer in column `column` is equal to `value`.

#### `IntBetween`: `{"column": string, "from": number | null, "to": number | null}`

This filter is true if the integer in column `column` is between `from` and `to`.
If from or to is `null` it will not constrain the value in that regard. `null` will always be excluded.

The `from` and `to` bounds are inclusive.

#### `FloatEquals`: `{"column": string, "value":string}`

This filter is true if the float in column `column` is equal to `value`.

#### `FloatBetween`: `{"column": string, "from": number | null, "to": number | null}`

This filter is true if the float in column `column` is between `from` and `to`.
If from or to is `null` it will not constrain the value in that regard. `null` will always be excluded.

The `from` bound is inclusive, the `to` bound exclusive.

## Action

This largely corresponds to the [LAPIS endpoint](https://lapis.cov-spectrum.org/open/v2/swagger-ui/index.html) that was being used. Always returns data as ndjson (every line is a separate json element and does not contain line-breaks)

Action body can always contain 
```
"orderByFields": 
    (string | 
    {"field": string, "order": "ascending" | "descending"})[]
"limit": number,
"offset": number,
"randomize": boolean | {"seed": number}
```

### `Aggregated`

`{"groupByFields": string[]}`

Aggregate (with count) the results by `groupByFields`. 

### `Details`

`{"fields": string[]}`

Return the fields `fields` of all filtered rows.

### `Mutations`

`{"fields": string[], , "minProportion": number, "sequenceNames": string | string[]}`
 
Finds all mutations in the sequences `sequenceNames` of all filtered rows (all symbols different to the reference, except N)

Then divide this count by the coverage for each position (count of non-N reads that cover this position).

For all mutation where this proportion is greater or equals to `minProportion`, return: 

```
{
    /** The formatted mutation. E.g. A2C */
    mutation: string;
    /** The symbol that was mutated from. E.g. A */
    mutationFrom: string;
    /** The symbol that was mutated to. E.g. C */
    mutationTo: string;
    /** The position of the mutation. E.g. 2 */
    position: number;
    /** The name of the sequence this mutation occurred in */
    sequenceName: string;
    /** The coverage at that position */
    coverage: number;
    /** The proportion calculated as count / coverage */
    proportion: number;
}
```

### `AminoAcidMutations`

See `Mutations`.

### `Fasta`

`{"sequenceNames": string[], "additionalFields": string[]}`

Returns the unalignedNucleotideSequences of `sequenceNames` (might be more than 1) and `additionalFields`.

### `FastaAligned`

`{"sequenceNames": string[], "additionalFields": string[]}`

Returns the alignedNucleotideSequences or alignedAminoAcidSequences of `sequenceNames` (might be more than 1) and `additionalFields`.

### `Insertions`

Gather all insertions that are contained in the data and aggregate them by the insertion value.

As an example, if these insertions are in the filtered data:
```
key, insertions
1,[1:ACG, 123:CCCA]
2,[1:ACAG]
3,[1:ACG, 123:CCCA]
4,[]
5,[1:ACG, 123:CCCAG]
6,[]
8,[]
9,[]
11,[]
14,[]
```
this action will return the following counts:
```
insertions,count
1:ACG,3
123:CCCA,2
1:ACAG,1
123:CCCAG,1
```

### AminoAcidInsertions

See `Insertions`.
