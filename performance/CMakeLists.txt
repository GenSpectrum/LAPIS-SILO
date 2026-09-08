function(add_benchmark BENCH_NAME)
    add_executable(${BENCH_NAME} ${BENCH_NAME}.cpp)
    target_link_libraries(${BENCH_NAME} rhydblib)
    target_include_directories(${BENCH_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/performance)
endfunction(add_benchmark)

# Tool that generates all benchmark input data.
add_benchmark(generate_test_data)

set(BENCHMARK_NAMES
    mutation_benchmark
    many_string_equals
    many_short_read_filters
    clustered_ingestion_benchmark
    nof_sequence_filter
    sequence_column_insert
    co_occurrence_benchmark
    real_data_mutations_benchmark
)
foreach(bench ${BENCHMARK_NAMES})
    add_benchmark(${bench})
endforeach()

# Aggregate target that just builds every benchmark. Running them is done by
# performance/run_benchmarks.sh, invoked from the `benchmarks` Makefile target.
add_custom_target(build_benchmarks)
add_dependencies(build_benchmarks ${BENCHMARK_NAMES})

# Benchmark input datasets, declared here rather than fetched from C++. The `benchmark_data` target
# materializes them under localTestData/performance/ with build-system change detection (re-fetched
# only when missing or when the declaration below changes). `make generateTestData` builds it.
include(${CMAKE_CURRENT_LIST_DIR}/BenchmarkData.cmake)

benchmark_dataset(wasap_mutation_coverage.ndjson.zst
    DOWNLOAD https://fsn1.your-objectstorage.com/silo-benchmarking/real_data_mutations_benchmark/wasap-mutation-coverage-5samples.ndjson.zst
    SHA256   51f4a3a8b3ca75e0da17b48175e0ef87b49ebf1e20f25604c64228c577d6e111)

get_property(_benchmark_datasets GLOBAL PROPERTY BENCHMARK_DATASETS)
add_custom_target(benchmark_data DEPENDS ${_benchmark_datasets})
