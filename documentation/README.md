# LAPIS-SILO

High-performance analytical database for sequence alignment data

## Sections

- [API Reference](api.md) — HTTP API for querying preprocessed outputs
- [Python Bindings](python_bindings.md) — Embedding SILO in-process with the `silodb` package
- [Input Format](input_format.md) — NDJSON input format for genomic sequence data
- [Query Reference](query_documentation.md) — Filter expressions and query syntax
- [Maybe Filter](maybe_documentation.md) — Semantics of the `Maybe` filter expression
- [Lineage Definitions](lineage_definitions.md) — Monophyletic search using lineage DAGs
- [Phylogenetic Queries](phylogenetic_queries.md) — Phylogeny-based query support
- [Incremental Preprocessing](incremental_preprocessing.md) — Appending data to an existing database
- [Data Directories](data_directories.md) — How SILO discovers and manages preprocessed database states

### Developer

- [Contributing](developer/contributing.md) — Building, testing, and contributing to SILO
- [Configuration System](developer/config.md) — How YAML, environment variables, and CLI args are merged
- [Logging](developer/logging.md) — Log levels and spdlog configuration
- [Sequence Storage](developer/sequence_storage.md) — Internal representation of genomic sequences
- [Serialization Version](developer/serialization_version.md) — Binary format versioning and how to bump it
