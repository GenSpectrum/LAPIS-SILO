This folder contains self-contained tests (all with their own respective main() function).

All .cpp-files in this folder are targets of the cmake project and can be configured, made, and executed from the home repo after building RhyDB, e.g.:
```shell
make build/Release/rhydb

cmake --build build/Release --target performance/mutation_benchmark

./build/Release/performance/mutation_benchmark
```

These binaries will provide some text output on the performance and can be profiled independently.

They are not unit tests as they can take more extensive time to execute.

## Test data

Benchmark datasets are produced under `localTestData/performance/` (gitignored); each benchmark reads
its dataset back from there. Prepare the data once before running any benchmark:

```shell
make generateTestData
```

This does two things: it runs `generate_test_data` to generate the synthetic datasets (several
gigabytes of NDJSON) locally, and it builds the `benchmark_data` CMake target to download the one
real-data slice used by `real_data_mutations_benchmark`. That slice is declared in
`performance/CMakeLists.txt` via `benchmark_dataset()` (URL + sha256), so the download is defined in
the build, not in C++, and is change-detected: it is fetched only when missing or when the declaration
changes, and skipped otherwise. Re-run `make generateTestData` when the generators in
`sequence_generator.h` change. If a benchmark is run before its data exists, it fails with a message
pointing back to `make generateTestData`. The synthetic dataset paths
are the `*_NDJSON_PATH` constants in `sequence_generator.h`.

To build and run every benchmark in one step (generating the data first if needed), use:

```shell
make benchmarks
```

This runs `performance/run_benchmarks.sh`, which executes each benchmark in sequence, continues past
any that fail, and reports which ones exited non-zero.

## Clustered ingestion (`clustered_ingestion_benchmark`)

`clustered_ingestion_benchmark` demonstrates what N-way clustered ingestion buffering buys. It models
amplicon/targeted sequencing: `generate_test_data` writes its reads as a fixed set of primer-defined
coverage windows (`DEFAULT_NUM_AMPLICONS` classes) in two on-disk orderings — amplicon-sorted and
randomly shuffled — that hold the exact same reads. A nucleotide-position filter only matches reads
of the one amplicon covering that position, so the query set is fast when those reads sit
contiguously in the ingested chunks (the coverage filter can skip chunks) and slow when they are
scattered across every chunk.

The benchmark builds the database three ways in one run and times the identical query set against
each:

1. amplicon-sorted input, clustering off — the ideal layout, for free;
2. amplicon-shuffled input, clustering off — worst case, coverage scattered everywhere;
3. amplicon-shuffled input, 128-way clustered ingestion — clustered buffering reorders it back.

The point is that (3) recovers the query performance of (1) from the same scattered input as (2). It
prints a summary of ingestion and query time per scenario; no environment variables or rebuilds are
needed to switch between them.

## Mutation-coverage query (`real_data_mutations_benchmark`)

`real_data_mutations_benchmark` times a co-occurrence `groupBy` over the ~141 real SARS-CoV-2 mutation
positions in `performance/mutations.csv`. It exercises the coverage-scan / per-chunk bitmap-aggregation
path over **short reads with partial coverage** (each read covers a small genome window, so most
grouped positions are not-covered for any given read) — the path that whole-genome-sequence datasets
do not stress.

It runs against real wastewater short-read data: a fixed slice of 5 whole samples (~11.3M reads,
in RhyDB ingest format) from the GenSpectrum W-ASAP dataset — the RhyDB instance behind
`db.wasap.genspectrum.org`. The slice is hosted publicly on Hetzner Object Storage and declared in
`performance/CMakeLists.txt`:

```cmake
benchmark_dataset(wasap_mutation_coverage.ndjson.zst
    DOWNLOAD <url>
    SHA256   <hex>)
```

`make generateTestData` builds the `benchmark_data` target, which invokes
`performance/ci/fetch_dataset.cmake` through CMake's script mode.
