# Legacy NDJSON Transformer

A command-line utility for transforming legacy NDJSON format genomic data into the format expected by SILO.

## Overview

This Rust tool transforms NDJSON (Newline Delimited JSON) files from the legacy format to the current SILO-compatible format. It processes genomic sequence data including aligned/unaligned nucleotide sequences, amino acid sequences, and their associated insertions.

## Input Format

The tool expects NDJSON input with the following structure:

```json
{
  "metadata": {
    "key": "sample_id",
    "col": "value"
  },
  "alignedNucleotideSequences": {
    "segment1": "ATCG..."
  },
  "unalignedNucleotideSequences": {
    "segment1": "ATCG..."
  },
  "alignedAminoAcidSequences": {
    "gene1": "MYKW..."
  },
  "nucleotideInsertions": {
    "segment1": ["123:A", "456:T"]
  },
  "aminoAcidInsertions": {
    "gene1": ["1:M", "2:Y"]
  }
}
```

## Output Format

The tool transforms the data into SILO's expected format:

```json
{
  "key": "sample_id",
  "col": "value",
  "segment1": {
    "seq": "ATCG...",
    "insertions": ["123:A", "456:T"]
  },
  "gene1": {
    "seq": "MYKW...",
    "insertions": ["1:M", "2:Y"]
  },
  "unaligned_segment1": "ATCG..."
}
```

## Building

Ensure you have Rust installed, then build the tool:

```bash
cargo build --release
```

## Usage

The tool reads from stdin and writes to stdout:

```bash
# Transform a file
cat legacy_data.ndjson | cargo run > transformed_data.ndjson

# Or using the built binary
cat legacy_data.ndjson | ./target/release/legacy-ndjson-transformer > transformed_data.ndjson

# Chain with other commands if the files are big
curl zstdcat input_file.old.ndjson.zst | ./target/release/legacy-ndjson-transformer | zstd > zstdcat input_file.ndjson.zst
```

## Testing

Run the test suite:

```bash
cargo test
```

## Transformation Rules

1. **Metadata fields** are copied directly to the output root level
2. **Aligned sequences** (nucleotide/amino acid) are transformed into objects with `seq` and `insertions` fields
3. **Unaligned nucleotide sequences** are prefixed with `unaligned_` and added as direct values
4. **Null sequences** remain as null values in the output
5. **Missing insertions** default to empty arrays

## Dependencies

- `serde` - Serialization/deserialization framework
- `serde_json` - JSON support for serde
