function(add_benchmark BENCH_NAME)
    add_executable(${BENCH_NAME} ${BENCH_NAME}.cpp)
    target_link_libraries(${BENCH_NAME} silolib)
    target_include_directories(${BENCH_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/performance)
endfunction(add_benchmark)

# Tool that generates all benchmark input data.
add_benchmark(generate_test_data)

set(BENCHMARK_NAMES
    mutation_benchmark
    many_string_equals
    many_short_read_filters
    nof_sequence_filter
    sequence_column_insert
    co_occurrence_benchmark
)
foreach(bench ${BENCHMARK_NAMES})
    add_benchmark(${bench})
endforeach()

# Aggregate target that just builds every benchmark. Running them is done by
# performance/run_benchmarks.sh, invoked from the `benchmarks` Makefile target.
add_custom_target(build_benchmarks)
add_dependencies(build_benchmarks ${BENCHMARK_NAMES})
