This folder contains self-contained tests (all with their own respective main() function).

All .cpp-files in this folder are targets of the cmake project and can be configured, made, and executed as such, e.g.:
```shell
cmake -B build/Release

cmake --build build/Release --target performance/mutation_benchmark

./build/Release/performance/mutation_benchmark
```

These binaries will provide some text output on the performance and can be profiled independently.

They are not unit tests as they can take more extensive time to execute.
