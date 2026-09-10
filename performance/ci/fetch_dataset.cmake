# Download a fixed benchmark-input artifact and verify its checksum, at build time.
#
# Invoked by the `benchmark_data` target (see performance/BenchmarkData.cmake) as:
#   cmake -DURL=<url> -DSHA256=<hex> -DDEST=<path> -P fetch_dataset.cmake
#
# Uses CMake's built-in verified download (no curl/sha256sum/bash needed). Idempotent: if DEST already
# exists with the expected hash it is a no-op, so re-running -- or re-building benchmark_data -- does
# not re-download.
if(NOT DEFINED URL OR NOT DEFINED SHA256 OR NOT DEFINED DEST)
    message(FATAL_ERROR "fetch_dataset.cmake requires -DURL= -DSHA256= -DDEST=")
endif()

if(EXISTS "${DEST}")
    file(SHA256 "${DEST}" _actual)
    if(_actual STREQUAL "${SHA256}")
        message(STATUS "benchmark dataset already present and verified: ${DEST}")
        return()
    endif()
endif()

message(STATUS "downloading benchmark dataset from ${URL}")
file(DOWNLOAD "${URL}" "${DEST}"
    EXPECTED_HASH SHA256=${SHA256}
    SHOW_PROGRESS
    STATUS _status)

list(GET _status 0 _code)
if(NOT _code EQUAL 0)
    file(REMOVE "${DEST}")
    list(GET _status 1 _message)
    message(FATAL_ERROR "download of ${URL} failed: ${_message}")
endif()
message(STATUS "downloaded and verified: ${DEST}")
