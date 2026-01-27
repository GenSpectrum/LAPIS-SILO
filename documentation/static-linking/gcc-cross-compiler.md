# Build with a GCC built for cross-compilation

As per [https://wiki.musl-libc.org/getting-started](https://wiki.musl-libc.org/getting-started), section "Building a cross compiler targeting musl libc":

    git clone https://github.com/richfelker/musl-cross-make/
    cd *
    make TARGET=x86_64-linux-musl install
    # ^ or rather with a --prefix option

Then:

    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ PATH=/home/coworking/src/musl-cross-make/output/bin/:"$PATH"
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ export CXX=x86_64-linux-musl-g++
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ export CC=x86_64-linux-musl-gcc
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ export LD=x86_64-linux-musl-ld

    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ rm build
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ rm conanprofile
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ rm /home/coworking/.conan2/profiles/default
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ make conanprofile
    (venv-silo)   0 coworking@chrdom1:~/SILO/LAPIS-SILO$ cat conanprofile
    [settings]
    arch=x86_64
    build_type=Release
    compiler=gcc
    compiler.cppstd=gnu14
    compiler.libcxx=libstdc++11
    compiler.version=9
    os=Linux

Leads to:

    ERROR: There are invalid packages:
    simdjson/3.12.3: Invalid: Current cppstd (gnu14) is lower than the required C++ standard (17).
    poco/1.13.3: Cannot build for this configuration: Current cppstd (gnu14) is lower than the required C++ standard (17).

Well, can't really do anything about that?, I'm already using the newest version of `https://github.com/richfelker/musl-cross-make/` and that decides which version of GCC to download itself.
