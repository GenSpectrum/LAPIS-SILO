# Declarative registry of benchmark input datasets.
#
# Each dataset is a file materialized under localTestData/performance/ from a declared source. It is
# produced by the build system (the `benchmark_data` target) only when it is missing or when its
# definition here changes, so data provenance (URLs, checksums) lives in the build definition rather
# than in C++, and change detection comes for free.
#
#   benchmark_dataset(<output-file-name>
#       DOWNLOAD <url> SHA256 <hex>   # fetch a fixed public artifact and verify its checksum
#   )
#
# A future GENERATE kind will produce the synthetic datasets via the generate_test_data tool (they
# still live in that tool for now); it would add an OUTPUT that DEPENDS on the generator binary so
# each synthetic dataset regenerates only when the generator changes.

set(BENCHMARK_DATA_DIR "${CMAKE_SOURCE_DIR}/localTestData/performance")
set(_BENCHMARK_DATA_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(benchmark_dataset name)
    cmake_parse_arguments(ARG "" "DOWNLOAD;SHA256" "" ${ARGN})
    if(NOT ARG_DOWNLOAD OR NOT ARG_SHA256)
        message(FATAL_ERROR "benchmark_dataset(${name}): requires DOWNLOAD <url> and SHA256 <hex>")
    endif()

    set(out "${BENCHMARK_DATA_DIR}/${name}")
    set(fetch "${_BENCHMARK_DATA_CMAKE_DIR}/ci/fetch_dataset.cmake")

    # The definition (url + sha) written to a stamp. file(CONFIGURE) rewrites it -- advancing its
    # timestamp -- only when the content changes, so the download below re-runs only when the url or
    # sha actually change (or when `out` is missing). An unchanged definition is a no-op.
    set(def "${CMAKE_BINARY_DIR}/benchmark_data/${name}.def")
    file(CONFIGURE OUTPUT "${def}" CONTENT "${ARG_DOWNLOAD}\n${ARG_SHA256}\n")

    # Download at build time via CMake's own verified download (file(DOWNLOAD ... EXPECTED_HASH ...));
    # no curl/sha256sum/bash dependency.
    add_custom_command(
        OUTPUT "${out}"
        COMMAND "${CMAKE_COMMAND}" "-DURL=${ARG_DOWNLOAD}" "-DSHA256=${ARG_SHA256}" "-DDEST=${out}"
                -P "${fetch}"
        DEPENDS "${def}" "${fetch}"
        COMMENT "benchmark data: fetching ${name}"
        VERBATIM
    )
    set_property(GLOBAL APPEND PROPERTY BENCHMARK_DATASETS "${out}")
endfunction()
