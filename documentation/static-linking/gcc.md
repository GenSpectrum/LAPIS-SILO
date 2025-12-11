# Build with normal GCC, but linking MUSL explicitly

MUSL provides a script `musl-gcc` that wraps gcc so that it links MUSL. But that only works for C, not C++.

There is no `musl-g++` wrapper.

There is a `musl-ldd` wrapper, though, try that.

    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ export LD=musl-ldd
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ unset CXX
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ unset CC
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ tra conanprofile ; tra build ; tra /home/coworking/.conan2/profiles/default
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ make conanprofile
    Detected profile:
    [settings]
    arch=x86_64
    build_type=Release
    compiler=gcc
    compiler.cppstd=gnu17
    compiler.libcxx=libstdc++11
    compiler.version=14
    os=Linux

    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ time VERBOSE=1 ./build_with_conan.py  --release --parallel 32

This succeeds compiling and linking, but:

    [100%] Linking CXX executable silo_test
    /usr/bin/cmake -E cmake_link_script CMakeFiles/silo_test.dir/link.txt --verbose=1
    /usr/bin/c++ ...

and in fact:

    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ ldd build/Release/silo
      linux-vdso.so.1 (0x00007f22ab056000)
      libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f22a8c00000)
      libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f22aaf41000)
      libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f22aaf12000)
      libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f22a8a0a000)
      /lib64/ld-linux-x86-64.so.2 (0x00007f22ab058000)

Not sure why `LD` is ignored. Cmake uses `c++` for linking, which in turn perhaps has the linker path hard-coded.
