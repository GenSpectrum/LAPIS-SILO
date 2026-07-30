This folder contains self-contained tests (all with their own respective main() function).

All .cpp-files in this folder are targets of the cmake project and can be configured, made, and executed from the home repo after building SILO, e.g.:
```shell
make build/Release/silo

cmake --build build/Release --target performance/mutation_benchmark

./build/Release/performance/mutation_benchmark
```

These binaries will provide some text output on the performance and can be profiled independently.

They are not unit tests as they can take more extensive time to execute.

## Test data

The benchmarks no longer generate their input data on every run. Instead, a single tool
(`generate_test_data`) produces all datasets once and writes them to disk under
`localTestData/performance/` (gitignored); each benchmark reads its dataset back from there. Generate
the data once before running any benchmark:

```shell
make generateTestData
```

This writes several gigabytes of NDJSON. Re-run it only when the generators in `sequence_generator.h`
change. If a benchmark is run before the data exists, it fails with a message pointing back to
`make generateTestData`. The dataset paths are defined as the `*_NDJSON_PATH` constants in
`sequence_generator.h`.

To build and run every benchmark in one step (generating the data first if needed), use:

```shell
make benchmarks
```

This runs `performance/run_benchmarks.sh`, which executes each benchmark in sequence, continues past
any that fail, and reports which ones exited non-zero.
