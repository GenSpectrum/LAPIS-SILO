# The WebAssembly (Emscripten) embind module. Like app/ and python/, this is a
# thin consumer of the portable rhydblib declared in the root CMakeLists.txt.
#
# This subdirectory is only added when configuring with the Emscripten toolchain
# (see the EMSCRIPTEN guard in the root CMakeLists.txt). The shared C++ standard,
# core dependencies, source globbing, and the NO_EVOBENCH define and
# -fexceptions/-pthread compile flags all come from rhydblib / rhydblib_deps.

add_executable(rhydb_wasm src/rhydb_wasm.cpp)
# Emit rhydb_wasm.js/.wasm at build/wasm/ (not build/wasm/wasm/) so the Makefile
# `wasm` target, dist copy, and node test find them at the historical path.
set_target_properties(rhydb_wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Linking the OBJECT library directly pulls in $<TARGET_OBJECTS:rhydblib> plus its
# usage requirements: the "src" include root, the core link libraries, and the
# Emscripten-only defines and compile flags carried by rhydblib_deps. rhydb_wasm is
# the only link step, so -fexceptions/-pthread are repeated here as link options.
target_link_libraries(rhydb_wasm PRIVATE rhydblib)

# 64-bit WebAssembly (MEMORY64). RHYDB_WASM_MEMORY64 is declared in the root
# CMakeLists.txt and, when ON, compiles rhydblib with -sMEMORY64=1. The final
# link must use the same ABI, so mirror the flag here. -sALLOW_MEMORY_GROWTH=1
# stays enabled in both variants; MEMORY64 raises the addressable ceiling above
# the 32-bit ~4 GB limit.
if(RHYDB_WASM_MEMORY64)
    set(RHYDB_WASM_MEMORY64_LINK_OPTION -sMEMORY64=1)
else()
    set(RHYDB_WASM_MEMORY64_LINK_OPTION)
endif()

target_link_options(rhydb_wasm
    PRIVATE
    -pthread
    -fexceptions
    -lembind
    ${RHYDB_WASM_MEMORY64_LINK_OPTION}
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
    -sUSE_PTHREADS=1
    -sENVIRONMENT=web,worker,node
)
