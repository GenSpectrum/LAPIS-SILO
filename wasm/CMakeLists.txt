# The WebAssembly (Emscripten) embind module. Like app/ and python/, this is a
# thin consumer of the portable silolib declared in the root CMakeLists.txt.
#
# This subdirectory is only added when configuring with the Emscripten toolchain
# (see the EMSCRIPTEN guard in the root CMakeLists.txt). The shared C++ standard,
# core dependencies, source globbing, and the NO_EVOBENCH define and
# -fexceptions/-pthread compile flags all come from silolib / silolib_deps.

add_executable(silo_wasm src/silo_wasm.cpp)
# Emit silo_wasm.js/.wasm at build/wasm/ (not build/wasm/wasm/) so the Makefile
# `wasm` target, dist copy, and node test find them at the historical path.
set_target_properties(silo_wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Linking the OBJECT library directly pulls in $<TARGET_OBJECTS:silolib> plus its
# usage requirements: the "src" include root, the core link libraries, and the
# Emscripten-only defines and compile flags carried by silolib_deps. silo_wasm is
# the only link step, so -fexceptions/-pthread are repeated here as link options.
target_link_libraries(silo_wasm PRIVATE silolib)

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
