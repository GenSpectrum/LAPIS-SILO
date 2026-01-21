function(add_benchmark BENCH_NAME)
    add_executable(${BENCH_NAME} ${BENCH_NAME}.cpp)
    target_link_libraries(${BENCH_NAME} silolib_obj)
endfunction(add_benchmark)

add_benchmark(mutation_benchmark)
add_benchmark(many_string_equals)
