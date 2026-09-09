# GTest is found at the top level only when BUILD_UNIT_TESTS is on; the benchmarks need it either way.
find_package(GTest REQUIRED)
if (NOT GTest_LIBRARIES)
    set(GTest_LIBRARIES gtest gmock)
endif ()

# All benchmarks are one gtest executable
# -> only gtest filters are used for defining which benchmark to run
file(GLOB BENCHMARK_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
list(REMOVE_ITEM BENCHMARK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/generate_test_data.cpp")

add_executable(rhydb_benchmark ${BENCHMARK_SOURCES})
target_link_libraries(rhydb_benchmark PRIVATE ${GTest_LIBRARIES} rhydblib)
target_include_directories(rhydb_benchmark PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# executable for generating synthetic test data
add_executable(generate_test_data generate_test_data.cpp)
target_link_libraries(generate_test_data PRIVATE rhydblib)
target_include_directories(generate_test_data PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Benchmark input datasets, declared here rather than fetched or generated from C++, and materialized
# by the `benchmark_data` target (`make generateTestData`). See BenchmarkData.cmake.
include(${CMAKE_CURRENT_LIST_DIR}/BenchmarkData.cmake)

# Synthetic datasets, written by the generate_test_data tool.
benchmark_dataset(short_reads_100k.ndjson GENERATE)
benchmark_dataset(short_reads_5m.ndjson GENERATE)
benchmark_dataset(short_reads_amplicon_sorted_5m.ndjson GENERATE)
benchmark_dataset(short_reads_amplicon_shuffled_5m.ndjson GENERATE)
benchmark_dataset(full_sequences_100k.ndjson GENERATE)
benchmark_dataset(sequence_column_sequences.ndjson GENERATE)
benchmark_dataset(mutation_reads.ndjson GENERATE)
benchmark_dataset(string_equals_records.ndjson GENERATE)
benchmark_dataset(co_occurrence_sequences.ndjson GENERATE)

# Real-data slice for real_data_mutations_benchmark.
benchmark_dataset(wasap_mutation_coverage.ndjson.zst
    DOWNLOAD https://fsn1.your-objectstorage.com/silo-benchmarking/real_data_mutations_benchmark/wasap-mutation-coverage-5samples.ndjson.zst
    SHA256   51f4a3a8b3ca75e0da17b48175e0ef87b49ebf1e20f25604c64228c577d6e111)

get_property(_benchmark_datasets GLOBAL PROPERTY BENCHMARK_DATASETS)
add_custom_target(benchmark_data DEPENDS ${_benchmark_datasets})
