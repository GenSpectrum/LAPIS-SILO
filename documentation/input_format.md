# SILO Input Format Documentation

This document describes the NDJSON input format used by SILO for ingesting genomic sequence data and associated metadata.

## Overview

SILO expects input data in **NDJSON format** (Newline Delimited JSON), where each line contains a complete JSON object representing one record/sequence. The format is parsed using the simdjson library for high-performance streaming.

## File Structure

- **Format**: NDJSON (one JSON object per line)
- **Line Delimiters**: Standard newline characters (`\n`)
- **Empty Lines**: Automatically skipped
- **Trailing Newline**: Optional (files without trailing newlines are supported)

### Example

```json
{"primaryKey":"seq_001","date":"2021-03-18","country":"Switzerland","age":54,"main":{"sequence":"ACGT","insertions":[]}}
{"primaryKey":"seq_002","date":"2021-04-13","country":"Germany","age":null,"main":{"sequence":"AAGN","insertions":["2:AC"]}}
```

## Configuration Files

### database_config.yaml

Defines the schema for your SILO database:

```yaml
schema:
  instanceName: my_database
  metadata:
    - name: primaryKey
      type: string
    - name: date
      type: date
    - name: country
      type: string
      generateIndex: true
    - name: pango_lineage
      type: string
      generateLineageIndex: pango.yaml
    - name: age
      type: int
    - name: qc_value
      type: float
    - name: is_complete
      type: boolean
  primaryKey: primaryKey

defaultNucleotideSequence: main
defaultAminoAcidSequence: E
```

**Metadata Field Options**:
- `name`: Field name (must match JSON key)
- `type`: One of `string`, `int`, `float`, `date`, `boolean`
- `generateIndex`: Set to `true` to create a bitmap index for fast equality lookups. This is only valid for `string` columns
- `generateLineageIndex`: Path to lineage definition file for hierarchical queries. This is only possible if `generateIndex` is also set

`defaultNucleotideSequence` and `defaultAminoAcidSequence` are optional and set the default sequence to be searched for, so that the sequence name can be omitted in queries.

### reference_genomes.json

Defines reference sequences for alignment:

```json
{
  "nucleotideSequences": [
    {
      "name": "main",
      "sequence": "ATTAAAGGTTTATACCTTCCCAG..."
    }
  ],
  "genes": [
    {
      "name": "E",
      "sequence": "MYSFVSEETGTLIVNSVLLFLAFVVFLLVTLAILTA..."
    }
  ]
}
```

## Column Types and Value Semantics

Each field in the NDJSON must correspond to a metadata field defined in `database_config.yaml`. 

Currently, but subject to change in the future, the sequence column types are inferred from a `reference_genomes.json`:
- SequenceColumn<Nucleotide> for each entry in `nucleotideSequences`
- SequenceColumn<AminoAcid> for each entry in `aminoAcidSequences`
- ZstdCompressedStringColumn for each entry in `nucleotideSequences`, prefixed with 'unaligned_'

The following column types are supported:

### STRING

String values for text data.

| Property | Value |
|----------|-------|
| JSON Type | String |
| Null | `null` |

```json
{"country": "Switzerland"}
{"country": null}
```
### ZSTD_COMPRESSED_STRING

String values for textual data which is stored compressed by a common dictionary.

| Property | Value |
|----------|-------|
| JSON Type | String |
| Null | `null` |

```json
{"unaligned_main": "ACGTACGTACGTACGT"}
{"unaligned_main": null}
```

### INT

32-bit signed integers.

| Property | Value |
|----------|-------|
| JSON Type | Integer |
| Null | `null` |
| Range | -2,147,483,648 to 2,147,483,647 |

```json
{"age": 54}
{"age": null}
```

### FLOAT

64-bit floating-point numbers.

| Property | Value |
|----------|-------|
| JSON Type | Number (integer or decimal) |
| Null | `null` |

```json
{"qc_value": 0.98}
{"qc_value": null}
```

### DATE

Date values in ISO 8601 format.

| Property | Value |
|----------|-------|
| JSON Type | String |
| Format | `YYYY-MM-DD` |
| Null | `null` or `""` (empty string) |

```json
{"date": "2021-03-18"}
{"date": null}
{"date": ""}
```

**Validation**:
- Month must be in range [1, 12]
- Day must be in range [1, 31]
- Invalid dates are logged as warnings and treated as null

### BOOLEAN

Boolean true/false values.

| Property | Value |
|----------|-------|
| JSON Type | Boolean |
| Null | `null` |

```json
{"is_complete": true}
{"is_complete": false}
{"is_complete": null}
```

### NUCLEOTIDE_SEQUENCE

DNA/RNA sequence data with optional insertions.

| Property | Value |
|----------|-------|
| JSON Type | Object |
| Null | `null` |

**Structure**:
```json
{
  "main": {
    "sequence": "ACGTACGT",
    "insertions": ["214:EPE", "185:AC"],
    "offset": 0
  }
}
```

**Fields**:
- `sequence`: The nucleotide sequence string
- `insertions`: Array of insertions in format `position:sequence`. `sequence` is inserted _after_ `position`. `position` 0 inserts _before the first symbol_. Valid positions are therefore in the range `[0, n]` where `n` is the length of the reference sequence.
- `offset` (optional): Integer offset into reference genome (default: 0)

**Valid Nucleotide Characters**:

| Character | Meaning |
|-----------|---------|
| `A` | Adenine |
| `C` | Cytosine |
| `G` | Guanine |
| `T` | Thymine |
| `N` | Unknown/ambiguous |
| `-` | Gap (deletion) |

**Insertion Format**:
- Format: `position:insertion_sequence`
- Example: `"214:ACGT"` means insertion of "ACGT" after position 214
- Position is 1-indexed
- Invalid insertions cause parsing errors

**Unaligned Sequences**:
Raw unaligned sequences can be provided with an `unaligned_` prefix:
```json
{
  "main": {"sequence": "ACGT", "insertions": []},
  "unaligned_main": "ACGTACGT"
}
```

### AMINO_ACID_SEQUENCE

Protein sequence data with optional insertions.

| Property | Value |
|----------|-------|
| JSON Type | Object |
| Null | `null` |

**Structure** (identical to nucleotide):
```json
{
  "E": {
    "sequence": "MYSF*",
    "insertions": ["214:EPE"],
    "offset": 0
  }
}
```

**Valid Amino Acid Characters**:

| Character | Meaning |
|-----------|---------|
| `A-Z` | Standard amino acids (21 total) |
| `*` | Stop codon |
| `X` | Unknown/ambiguous |
| `-` | Gap (deletion) |

## Complete Example

### database_config.yaml

```yaml
schema:
  instanceName: example_database
  metadata:
    - name: accession
      type: string
    - name: date
      type: date
    - name: country
      type: string
      generateIndex: true
    - name: age
      type: int
    - name: qc_score
      type: float
  primaryKey: accession

defaultNucleotideSequence: main
```

### Input NDJSON

```json
{"accession":"SEQ001","date":"2021-03-18","country":"Switzerland","age":54,"qc_score":0.98,"main":{"sequence":"ACGTACGT","insertions":[]}}
{"accession":"SEQ002","date":"2021-04-13","country":"Germany","age":null,"qc_score":0.95,"main":{"sequence":"AAGNAAGN","insertions":["4:CC"]}}
{"accession":"SEQ003","date":null,"country":"France","age":32,"qc_score":null,"main":{"sequence":"ACGTNNNN","insertions":[]}}
```

## Error Handling

| Error | Cause | Behavior |
|-------|-------|----------|
| Missing required field | Field in schema not present in JSON | Error with field name |
| Type mismatch | JSON value type doesn't match schema | Error with details |
| Invalid date format | Date string not in YYYY-MM-DD format | Warning, treated as null |
| Invalid sequence character | Unknown nucleotide/amino acid | Error with position |
| Invalid insertion format | Malformed insertion string | Error with details |
| Duplicate primary key | Same primary key appears twice | Error at validation |
| Unknown field | Field in JSON not in schema | Warning, ignored |
