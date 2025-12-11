# Attempt at building SILO on Alpine Linux

The idea here is that Alpine uses musl as its system libc, thus adding the `-static` flag is all that's needed. Building in an Alpine docker image (or using `alpine-standard-3.23.0-x86_64.iso` in QEMU-KVM as I did here) would be all that's needed.

Install system dependencies (some may be redundant):

    alp:~# apk add bash git python3 py3-pip cmake gcc make g++ build-base perl linux-headers

    ((.venv) ) alp:~/LAPIS-SILO$ python3 -mvenv .venv
    ((.venv) ) alp:~/LAPIS-SILO$ source ~/.venv/bin/activate
    ((.venv) ) alp:~/LAPIS-SILO$ pip install conan
    ((.venv) ) alp:~/LAPIS-SILO$ make conanprofile
    ((.venv) ) alp:~/LAPIS-SILO$ ./build_with_conan.py --parallel 32 --release

Led to this problem:

    boost/1.85.0: RUN: b2 -q target-os=linux architecture=x86 address-model=64 binary-format=elf abi=sysv --layout=system --user-config=/home/chris/.conan2/p/boost63dd5e7c53f47/s/src/tools/build/user-config.jam -sNO_ZLIB=0 -sNO_BZIP2=0 -sNO_LZMA=0 -sNO_ZSTD=0 boost.locale.icu=off --disable-icu boost.locale.iconv=off --disable-iconv define=LZMA_API_STATIC threading=multi visibility=hidden link=static variant=release --with-charconv --with-container --with-iostreams --with-random --with-regex --with-serialization --with-system --with-url toolset=gcc cxxstd=17 cxxstd-dialect=gnu define=_GLIBCXX_USE_CXX11_ABI=1 pch=on linkflags="" cxxflags="-fPIC" install --prefix=/home/chris/.conan2/p/b/boost2466d0b4b394b/p -j32 --abbreviate-paths -d0 --debug-configuration --build-dir="/home/chris/.conan2/p/b/boost2466d0b4b394b/b/build-release"
    /bin/sh: b2: not found

    boost/1.85.0: ERROR: 
    Package '1bd1dafb06d6e4883ef7e41c9fe29d623e7edb4a' build failed

That turns out to *not* be related to the b2 tool that Debian has in `libc6-dbg`, and also *not* the one that Alpine has in `b2-tools` (which is "Command-line tool for Backblaze B2"), but yet a completely different thing of the same name that boost uses *internally*. GPT knows something about that:

    That means the Boost build system (`b2`, also known as **bjam**) is not available in your Alpine environment when Conan tries to build Boost. Boost’s build process requires `b2`, which is normally built from the `boost-build` sources before compiling the libraries.

    - If dependencies like `bash`, `make`, `python3`, or `build-base` are missing, the `b2` bootstrap step fails silently, leaving no `b2` binary to run.

But those *were* installed. But this helped:

    cd ~/.conan2/p/boost*/s/src/tools/build
    ./bootstrap.sh gcc
    ./b2 --version

    add it to your `PATH`

Then I got to this problem (longer output further below):

    CMake Error: Could not find CMAKE_ROOT !!!
    CMake has most likely not been installed correctly.

GPT told me that this is because it is using cmake from conan instead of the system cmake, thus:

    ((.venv) ) alp:~/LAPIS-SILO$ conan remove cmake 
    Found 2 pkg/version recipes matching cmake in local cache
    Remove the recipe and all the packages of 'cmake/3.31.10#313d16a1aa16bbdb2ca0792467214b76'? (yes/no): yes
    Remove the recipe and all the packages of 'cmake/4.2.0#ae0a44f44a1ef9ab68fd4b3e9a1f8671'? (yes/no): yes
    Remove summary:
    Local Cache
      cmake/3.31.10#313d16a1aa16bbdb2ca0792467214b76: Removed recipe and all binaries
      cmake/4.2.0#ae0a44f44a1ef9ab68fd4b3e9a1f8671: Removed recipe and all binaries

    ((.venv) ) alp:~/LAPIS-SILO$ cmake --version
    cmake version 4.1.3

    CMake suite maintained and supported by Kitware (kitware.com/cmake).
    ((.venv) ) alp:~/LAPIS-SILO$ which cmake
    /usr/bin/cmake

This is still the installed version from Alpine.

    alp:~# apk info cmake
    cmake-4.1.3-r0 description:
    Cross-platform, open-source make system
    ...

Apparently some dependency of SILO is not happy with that, since it is actively re-installing cmake again, and promptly failing again:

    ((.venv) ) alp:~/LAPIS-SILO$ ./build_with_conan.py --parallel 32 --release
    ----------------------------------
    conan install . --update --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build/Release/generators --settings 'arrow/*:compiler.cppstd=20'
    ----------------------------------

    ======== Input profiles ========
    Profile host:
    [settings]
    arch=x86_64
    build_type=Release
    compiler=gcc
    compiler.cppstd=gnu17
    compiler.libcxx=libstdc++11
    compiler.version=15
    os=Linux
    arrow/*:compiler.cppstd=20

    Profile build:
    [settings]
    arch=x86_64
    build_type=Release
    compiler=gcc
    compiler.cppstd=gnu17
    compiler.libcxx=libstdc++11
    compiler.version=15
    os=Linux

    ...

    A new experimental approach for binary compatibility detection is available.
        Enable it by setting the core.graph:compatibility_mode=optimized conf
        and get improved performance when querying multiple compatible binaries in remotes.

    ...

    ======== Installing packages ========

    -------- Downloading 2 packages --------
    Downloading binary packages in 32 parallel threads
    cmake/3.31.10: Retrieving package 63fead0844576fc02943e16909f08fcdddd6f44b from remote 'conancenter' 
    cmake/4.2.0: Retrieving package 63fead0844576fc02943e16909f08fcdddd6f44b from remote 'conancenter' 
    cmake/3.31.10: Downloading 45.1MB conan_package.tgz
    cmake/4.2.0: Downloading 47.0MB conan_package.tgz
    cmake/3.31.10: Downloaded 28.8MB 63% conan_package.tgz
    cmake/4.2.0: Downloaded 29.2MB 62% conan_package.tgz
    cmake/3.31.10: Decompressing 45.1MB conan_package.tgz
    cmake/4.2.0: Decompressing 47.0MB conan_package.tgz
    cmake/3.31.10: Package installed 63fead0844576fc02943e16909f08fcdddd6f44b
    cmake/3.31.10: Downloaded package revision 8b53d1fe96d4b000f07eeaf7513ffdb4
    cmake/4.2.0: Package installed 63fead0844576fc02943e16909f08fcdddd6f44b
    cmake/4.2.0: Downloaded package revision 5b185fe408a8f0aee7d2012977f641b5
    abseil/20240116.1: Already installed! (1 of 28)
    bzip2/1.0.8: Already installed! (2 of 28)
    fmt/11.1.3: Already installed! (5 of 28)
    gtest/1.16.0: Already installed! (6 of 28)
    m4/1.4.19: Already installed! (7 of 28)
    nlohmann_json/3.12.0: Already installed! (8 of 28)
    rapidjson/cci.20230929: Already installed! (9 of 28)
    roaring/4.2.1: Already installed! (10 of 28)
    simdjson/3.12.3: Already installed! (11 of 28)
    xsimd/13.0.0: Already installed! (12 of 28)
    xz_utils/5.8.1: Already installed! (13 of 28)
    yaml-cpp/0.8.0: Already installed! (14 of 28)
    zlib/1.3.1: Already installed! (15 of 28)
    zstd/1.5.7: Already installed! (16 of 28)
    boost/1.85.0: Already installed! (17 of 28)
    flex/2.6.4: Already installed! (18 of 28)

    -------- Installing package mimalloc/2.2.4 (19 of 28) --------
    mimalloc/2.2.4: Building from source
    mimalloc/2.2.4: Package mimalloc/2.2.4:98911e64742bd608cd34714d9ff1d9603d38f672
    mimalloc/2.2.4: settings: os=Linux arch=x86_64 compiler=gcc compiler.cppstd=gnu17 compiler.libcxx=libstdc++11 compiler.version=15 build_type=Release
    mimalloc/2.2.4: options: fPIC=True guarded=False override=True secure=False shared=False single_object=False
    mimalloc/2.2.4: Copying sources to build folder
    mimalloc/2.2.4: Building your package in /home/chris/.conan2/p/b/mimale72424be55a5e/b
    mimalloc/2.2.4: Calling generate()
    mimalloc/2.2.4: Generators folder: /home/chris/.conan2/p/b/mimale72424be55a5e/b/build/Release/generators
    mimalloc/2.2.4: CMakeToolchain generated: conan_toolchain.cmake
    mimalloc/2.2.4: CMakeToolchain generated: /home/chris/.conan2/p/b/mimale72424be55a5e/b/build/Release/generators/CMakePresets.json
    mimalloc/2.2.4: CMakeToolchain generated: /home/chris/.conan2/p/b/mimale72424be55a5e/b/src/CMakeUserPresets.json
    mimalloc/2.2.4: Generating aggregated env files
    mimalloc/2.2.4: Generated aggregated env files: ['conanbuild.sh', 'conanrun.sh']
    mimalloc/2.2.4: Calling build()
    mimalloc/2.2.4: Apply patch (portability): fix compilation errors on older compilers
    mimalloc/2.2.4: Running CMake.configure()
    mimalloc/2.2.4: RUN: cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="generators/conan_toolchain.cmake" -DCMAKE_INSTALL_PREFIX="/home/chris/.conan2/p/b/mimale72424be55a5e/p" -DCMAKE_POLICY_DEFAULT_CMP0091="NEW" -DCMAKE_BUILD_TYPE="Release" "/home/chris/.conan2/p/b/mimale72424be55a5e/b/src"
    CMake Error: Could not find CMAKE_ROOT !!!
    CMake has most likely not been installed correctly.
    Modules directory not found in
    /home/chris/.conan2/p/cmake83736d917166f/p/share/cmake
    CMake Error: Error executing cmake::LoadCache(). Aborting.

GPT claimed I could tell conan `tools.cmake.cmaketool:system_cmake=True` to override, but theres no such variable in 

    ((.venv) ) alp:~/LAPIS-SILO$ conan --version
    Conan version 2.23.0
    ((.venv) ) alp:~/LAPIS-SILO$ conan config list|grep -i cmake|grep -i system
    tools.cmake.cmaketoolchain:system_name: Define CMAKE_SYSTEM_NAME in CMakeToolchain
    tools.cmake.cmaketoolchain:system_processor: Define CMAKE_SYSTEM_PROCESSOR in CMakeToolchain
    tools.cmake.cmaketoolchain:system_version: Define CMAKE_SYSTEM_VERSION in CMakeToolchain
    tools.cmake.cmaketoolchain:toolset_cuda: (Experimental) Path to a CUDA toolset to use, or version if installed at the system level

Not sure how to get the build going without being able to side step the non-working dependency. Or why even conan's cmake are broken on Alpine.
