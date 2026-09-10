# Declarative registry of benchmark input datasets.
#
# Each dataset is a file materialized under localTestData/performance/ from a declared source:
#
#   benchmark_dataset(<file-name> GENERATE)                    # written by the generate_test_data tool
#   benchmark_dataset(<file-name> DOWNLOAD <url> SHA256 <hex>) # fetched and checksum-verified
#
if(CMAKE_SCRIPT_MODE_FILE)
    # Idempotent download of one dataset, run at build time: a file that is already there
    # with the expected hash is left alone, so re-building `benchmark_data` does not re-download.
    if(EXISTS "${DEST}")
        file(SHA256 "${DEST}" actual_hash)
        if(actual_hash STREQUAL "${SHA256}")
            message(STATUS "benchmark dataset already present and verified: ${DEST}")
            return()
        endif()
    endif()
    # Download and move to guard against interrupted downloads
    message(STATUS "downloading benchmark dataset from ${URL}")
    get_filename_component(dest_dir "${DEST}" DIRECTORY)
    file(MAKE_DIRECTORY "${dest_dir}")
    file(DOWNLOAD "${URL}" "${DEST}.part" EXPECTED_HASH SHA256=${SHA256} SHOW_PROGRESS STATUS status)
    # A failed download (network error or hash mismatch) must not leave a stale .part behind, or the
    # next attempt would try to rename a broken file into place.
    list(GET status 0 code)
    if(NOT code EQUAL 0)
        file(REMOVE "${DEST}.part")
        list(GET status 1 message)
        message(FATAL_ERROR "download of ${URL} failed: ${message}")
    endif()
    file(RENAME "${DEST}.part" "${DEST}")
    return()
endif()

set(BENCHMARK_DATA_DIR "${CMAKE_SOURCE_DIR}/localTestData/performance")
set(_BENCHMARK_DATA_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")

# Dataset generation dependencies to retrigger on changes
set(_generator_inputs
    "${CMAKE_CURRENT_LIST_DIR}/generate_test_data.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sequence_generator.h"
    "${CMAKE_SOURCE_DIR}/testBaseData/exampleDataset/reference_genomes.json"
)
set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${_generator_inputs})
foreach(input IN LISTS _generator_inputs)
    file(SHA256 "${input}" hash)
    string(APPEND _BENCHMARK_GENERATOR_DEFINITION "${input} ${hash}\n")
endforeach()

# Produce one dataset at a time
set_property(GLOBAL APPEND PROPERTY JOB_POOLS benchmark_data=1)

function(benchmark_dataset name)
    cmake_parse_arguments(ARG "GENERATE" "DOWNLOAD;SHA256" "" ${ARGN})

    set(out "${BENCHMARK_DATA_DIR}/${name}")
    if(ARG_GENERATE AND NOT ARG_DOWNLOAD)
        set(definition "${_BENCHMARK_GENERATOR_DEFINITION}")
        set(command generate_test_data "${name}")
        set(verb "generating")
    elseif(ARG_DOWNLOAD AND ARG_SHA256 AND NOT ARG_GENERATE)
        set(definition "${ARG_DOWNLOAD} ${ARG_SHA256}")
        set(command "${CMAKE_COMMAND}" "-DURL=${ARG_DOWNLOAD}" "-DSHA256=${ARG_SHA256}"
                    "-DDEST=${out}" -P "${_BENCHMARK_DATA_CMAKE_FILE}")
        set(verb "fetching")
    else()
        message(FATAL_ERROR
            "benchmark_dataset(${name}): pass either GENERATE or DOWNLOAD <url> with SHA256 <hex>")
    endif()

    # The dataset's definition, written to a stamp. file(CONFIGURE) rewrites it -- advancing its
    # timestamp -- only when the content changes, so the command below re-runs only when the
    # definition actually changed (or when `out` is missing). An unchanged definition is a no-op.
    set(def "${CMAKE_BINARY_DIR}/benchmark_data/${name}.def")
    file(CONFIGURE OUTPUT "${def}" CONTENT "${definition}\n")

    add_custom_command(
        OUTPUT "${out}"
        COMMAND ${command}
        DEPENDS "${def}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        JOB_POOL benchmark_data
        COMMENT "benchmark data: ${verb} ${name}"
        VERBATIM
    )
    set_property(GLOBAL APPEND PROPERTY BENCHMARK_DATASETS "${out}")
endfunction()
