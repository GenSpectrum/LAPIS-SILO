# The WebAssembly (Emscripten) embind module. Like app/ and python/, this is a
# thin consumer of the portable rhydblib declared in src/CMakeLists.txt.
#
# This subdirectory is only added with BUILD_WASM_MODULE (see the wasm preset),
# which needs the Emscripten toolchain. The shared C++ standard,
# core dependencies, source globbing, and the NO_EVOBENCH define all come from
# rhydblib, -pthread/-fexceptions from the toolchain.

add_executable(rhydb_wasm src/rhydb_wasm.cpp)
# Emit rhydb_wasm.js/.wasm at build/wasm/ (not build/wasm/wasm/) so the Makefile
# `wasm` target, dist copy, and node test find them at the historical path.
set_target_properties(rhydb_wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Linking the OBJECT library directly pulls in $<TARGET_OBJECTS:rhydblib> plus its
# usage requirements: the "src" include root, the core link libraries, and the
# Emscripten-only NO_EVOBENCH define.
target_link_libraries(rhydb_wasm PRIVATE rhydblib)

# 64-bit WebAssembly (wasm64). RHYDB_WASM_MEMORY64 is set by the wasm64 preset.
# The --target=wasm64 ABI flag reaches both compile and link via the Conan
# profile and the preset, so the only thing left to set here is the memory ceiling.
#
# -sALLOW_MEMORY_GROWTH lets memory grow up to MAXIMUM_MEMORY, which defaults to
# 2 GiB. For wasm64 that default would negate the point of the 64-bit build, so
# raise the ceiling past 4 GiB. The 32-bit build cannot address more than 4 GiB,
# so it keeps the default cap.
if(RHYDB_WASM_MEMORY64)
    set(RHYDB_WASM_MEMORY64_LINK_OPTIONS -sMAXIMUM_MEMORY=16GB)
else()
    set(RHYDB_WASM_MEMORY64_LINK_OPTIONS)
endif()

target_link_options(rhydb_wasm
    PRIVATE
    -lembind
    ${RHYDB_WASM_MEMORY64_LINK_OPTIONS}
    -sALLOW_MEMORY_GROWTH=1
    -sEXIT_RUNTIME=0
    -sEXPORT_ES6=1
    -sEXPORT_NAME=createRhydbModule
    # Emit TypeScript declarations (rhydb_wasm.d.ts) next to the module.
    --emit-tsd
    rhydb_wasm.d.ts
    -sEXPORTED_RUNTIME_METHODS=FS,getExceptionMessage
    -sFORCE_FILESYSTEM=1
    -sMODULARIZE=1
    -sPTHREAD_POOL_SIZE=4
    -sENVIRONMENT=web,worker,node
)
