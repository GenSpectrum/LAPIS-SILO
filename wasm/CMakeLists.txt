cmake_minimum_required(VERSION 3.22)
project(SILO_WASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SILO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/generators")
set(absl_DIR "${CMAKE_BINARY_DIR}")
set(Arrow_DIR "${CMAKE_BINARY_DIR}")
set(Boost_DIR "${CMAKE_BINARY_DIR}")
set(BZip2_DIR "${CMAKE_BINARY_DIR}")
set(FastFloat_DIR "${CMAKE_BINARY_DIR}")
set(fmt_DIR "${CMAKE_BINARY_DIR}")
set(Libevent_DIR "${CMAKE_BINARY_DIR}")
set(nlohmann_json_DIR "${CMAKE_BINARY_DIR}")
set(OpenSSL_DIR "${CMAKE_BINARY_DIR}")
set(re2_DIR "${CMAKE_BINARY_DIR}")
set(roaring_DIR "${CMAKE_BINARY_DIR}")
set(simdjson_DIR "${CMAKE_BINARY_DIR}")
set(simdutf_DIR "${CMAKE_BINARY_DIR}")
set(spdlog_DIR "${CMAKE_BINARY_DIR}")
set(Thrift_DIR "${CMAKE_BINARY_DIR}")
set(yaml-cpp_DIR "${CMAKE_BINARY_DIR}")
set(ZLIB_DIR "${CMAKE_BINARY_DIR}")
set(zstd_DIR "${CMAKE_BINARY_DIR}")

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
add_compile_definitions(SIMDJSON_EXCEPTIONS=0)
add_compile_definitions(SILO_DEBUG_ASSERTIONS=0)
add_compile_definitions(NO_EVOBENCH)

find_package(Arrow REQUIRED)
find_package(Boost REQUIRED COMPONENTS system serialization iostreams)
find_package(FastFloat REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(re2 REQUIRED)
find_package(roaring REQUIRED)
find_package(spdlog REQUIRED)
find_package(simdjson REQUIRED)
find_package(simdutf REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(zstd REQUIRED)

file(GLOB_RECURSE SILO_CORE_SOURCES CONFIGURE_DEPENDS "${SILO_ROOT}/src/*.cpp")

foreach(pattern
    "${SILO_ROOT}/src/*.test.cpp"
    "${SILO_ROOT}/src/main.cpp"
    "${SILO_ROOT}/src/silo/api/*.cpp"
    "${SILO_ROOT}/src/silo/append/append.cpp"
    "${SILO_ROOT}/src/silo/config/append_config.cpp"
    "${SILO_ROOT}/src/silo/config/runtime_config.cpp"
    "${SILO_ROOT}/src/evobench/evobench.cpp"
)
    file(GLOB_RECURSE MATCHING_SOURCES CONFIGURE_DEPENDS ${pattern})
    list(REMOVE_ITEM SILO_CORE_SOURCES ${MATCHING_SOURCES})
endforeach()

add_executable(silo_wasm
    src/silo_wasm.cpp
    ${SILO_CORE_SOURCES}
)

target_include_directories(silo_wasm PRIVATE "${SILO_ROOT}/src")

target_compile_definitions(silo_wasm PRIVATE SILO_WASM)

target_compile_options(silo_wasm PRIVATE -fexceptions -pthread)

target_link_libraries(silo_wasm
    PRIVATE
    Arrow::arrow_static
    ArrowAcero::arrow_acero_static
    ${Boost_LIBRARIES}
    ${FastFloat_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${roaring_LIBRARIES}
    ${spdlog_LIBRARIES}
    ${simdjson_LIBRARIES}
    ${simdutf_LIBRARIES}
    ${yaml-cpp_LIBRARIES}
    zstd::libzstd_static
    re2::re2
)

target_link_options(silo_wasm
    PRIVATE
    -pthread
    -fexceptions
    -lembind
    -sALLOW_MEMORY_GROWTH=1
    -sEXIT_RUNTIME=0
    -sEXPORT_ES6=1
    -sEXPORT_NAME=createSiloModule
    -sEXPORTED_RUNTIME_METHODS=FS,getExceptionMessage
    -sFORCE_FILESYSTEM=1
    -sMODULARIZE=1
    -sPTHREAD_POOL_SIZE=4
    -sUSE_PTHREADS=1
    -sENVIRONMENT=web,worker,node
)
