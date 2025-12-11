# Building via `clang++ -target x86_64-linux-musl -static`

LLVM can cross-compile by default, choosing the target at runtime (unlike GCC which only supports one target, chosen at its own compile time). This allows to choose musl for linking, easily.

Problem 1: how to use clang.

    LAPIS-SILO$ cat conanprofile
    [settings]
    arch=x86_64
    build_type=Release
    compiler=clang
    compiler.cppstd=gnu17
    compiler.libcxx=libstdc++11
    compiler.version=19
    os=Linux

This alone does not suffice, (some) dependencies are still compiled via gcc.

Thus (using `clang++-19` from Debian's `clang-19` package):

    export CXX=clang++-19

Now everything compiles via clang.

But we have another problem, that I haven't found a solution for:

    -------- Installing package arrow/22.0.0 (24 of 24) --------
    arrow/22.0.0: Building from source
    arrow/22.0.0: Package arrow/22.0.0:47bfb942dcd8247dce2a00ff6acd84c2b9f5bec6
    arrow/22.0.0: settings: os=Linux arch=x86_64 compiler=clang compiler.cppstd=20 compiler.libcxx=libstdc++11 compiler.version=19 build_type=Release
    arrow/22.0.0: options: acero=True cli=False compute=True dataset_modules=False deprecated=True encryption=False fPIC=True filesystem_layer=True gandiva=False hdfs_bridgs=False parquet=True plasma=deprecated runtime_simd_level=max shared=False simd_level=default substrait=False with_backtrace=False with_boost=True with_brotli=False with_bz2=False with_csv=False with_cuda=False with_flight_rpc=False with_flight_sql=False with_gcs=False with_gflags=False with_glog=False with_grpc=False with_jemalloc=False with_json=False with_llvm=False with_lz4=False with_mimalloc=False with_openssl=False with_opentelemetry=False with_orc=False with_protobuf=False with_re2=False with_s3=False with_snappy=False with_thrift=True with_utf8proc=False with_zlib=True with_zstd=False
    arrow/22.0.0: requires: thrift/0.20.Z libevent/2.1.Z openssl/3.6.Z boost/1.85.Z bzip2/1.0.Z xz_utils/5.8.Z zstd/1.5.Z rapidjson/cci xsimd/13.0.0#3e7543dc526b0f612fe291e0f198c9dc:da39a3ee5e6b4b0d3255bfef95601890afd80709 zlib/1.3.Z
    arrow/22.0.0: Copying sources to build folder
    arrow/22.0.0: Building your package in /home/coworking/.conan2/p/b/arrow25f5755664dfb/b
    arrow/22.0.0: Calling generate()
    arrow/22.0.0: Generators folder: /home/coworking/.conan2/p/b/arrow25f5755664dfb/b/build/Release/generators
    arrow/22.0.0: CMakeToolchain generated: conan_toolchain.cmake
    arrow/22.0.0: CMakeToolchain generated: /home/coworking/.conan2/p/b/arrow25f5755664dfb/b/build/Release/generators/CMakePresets.json
    arrow/22.0.0: Generating aggregated env files
    arrow/22.0.0: Generated aggregated env files: ['conanbuild.sh', 'conanrun.sh']
    arrow/22.0.0: Calling build()
    arrow/22.0.0: Running CMake.configure()
    arrow/22.0.0: RUN: cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="generators/conan_toolchain.cmake" -DCMAKE_INSTALL_PREFIX="/home/coworking/.conan2/p/b/arrow25f5755664dfb/p" -DARROW_SIMD_LEVEL="DEFAULT" -DARROW_RUNTIME_SIMD_LEVEL="MAX" -DCMAKE_PROJECT_arrow_INCLUDE="/home/coworking/.conan2/p/b/arrow25f5755664dfb/b/src/conan_cmake_project_include.cmake" -DCMAKE_POLICY_DEFAULT_CMP0091="NEW" -DCMAKE_BUILD_TYPE="Release" "/home/coworking/.conan2/p/b/arrow25f5755664dfb/b/src/cpp"
    Re-run cmake no build system arguments
    -- Building using CMake version: 3.31.10
    -- Using Conan toolchain: /home/coworking/.conan2/p/b/arrow25f5755664dfb/b/build/Release/generators/conan_toolchain.cmake
    -- Conan toolchain: Setting CMAKE_POSITION_INDEPENDENT_CODE=ON (options.fPIC)
    -- Conan toolchain: Defining architecture flag: -m64
    -- Conan toolchain: Defining libcxx as C++ flags: -stdlib=libstdc++
    -- Conan toolchain: C++ Standard 20 with extensions OFF
    -- Conan toolchain: Setting BUILD_SHARED_LIBS = OFF
    -- The C compiler identification is GNU 14.2.0
    -- The CXX compiler identification is Clang 19.1.7
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Check for working C compiler: /usr/bin/cc - skipped
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Check for working CXX compiler: /usr/bin/clang++-19 - skipped
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Arrow version: 22.0.0 (full: '22.0.0')
    -- Arrow SO version: 2200 (full: 2200.0.0)
    -- clang-tidy 14 not found
    -- clang-format 14 not found
    -- Could NOT find ClangTools (missing: CLANG_FORMAT_BIN CLANG_TIDY_BIN) 
    -- infer not found
    -- Found Python3: /home/coworking/venv-silo/bin/python3.12 (found version "3.12.2") found components: Interpreter
    -- System processor: x86_64
    -- Performing Test CXX_SUPPORTS_SSE4_2
    -- Performing Test CXX_SUPPORTS_SSE4_2 - Success
    -- Performing Test CXX_SUPPORTS_AVX2
    -- Performing Test CXX_SUPPORTS_AVX2 - Success
    -- Performing Test CXX_SUPPORTS_AVX512
    -- Performing Test CXX_SUPPORTS_AVX512 - Success
    -- Arrow build warning level: PRODUCTION
    -- Using ld linker
    -- Build Type: Release
    -- Performing Test CXX_LINKER_SUPPORTS_VERSION_SCRIPT
    -- Performing Test CXX_LINKER_SUPPORTS_VERSION_SCRIPT - Success
    -- Using SYSTEM approach to find dependencies
    -- ARROW_ABSL_BUILD_VERSION: 20211102.0
    -- ARROW_ABSL_BUILD_SHA256_CHECKSUM: dcf71b9cba8dc0ca9940c4b316a0c796be8fab42b070bb6b7cab62b48f0e66c4
    -- ARROW_AWS_C_AUTH_BUILD_VERSION: v0.9.0
    -- ARROW_AWS_C_AUTH_BUILD_SHA256_CHECKSUM: aa6e98864fefb95c249c100da4ae7aed36ba13a8a91415791ec6fad20bec0427
    -- ARROW_AWS_C_CAL_BUILD_VERSION: v0.9.2
    -- ARROW_AWS_C_CAL_BUILD_SHA256_CHECKSUM: f9f3bc6a069e2efe25fcdf73e4d2b16b5608c327d2eb57c8f7a8524e9e1fcad0
    -- ARROW_AWS_C_COMMON_BUILD_VERSION: v0.12.4
    -- ARROW_AWS_C_COMMON_BUILD_SHA256_CHECKSUM: 0b7705a4d115663c3f485d353a75ed86e37583157585e5825d851af634b57fe3
    -- ARROW_AWS_C_COMPRESSION_BUILD_VERSION: v0.3.1
    -- ARROW_AWS_C_COMPRESSION_BUILD_SHA256_CHECKSUM: d89fca17a37de762dc34f332d2da402343078da8dbd2224c46a11a88adddf754
    -- ARROW_AWS_C_EVENT_STREAM_BUILD_VERSION: v0.5.4
    -- ARROW_AWS_C_EVENT_STREAM_BUILD_SHA256_CHECKSUM: cef8b78e362836d15514110fb43a0a0c7a86b0a210d5fe25fd248a82027a7272
    -- ARROW_AWS_C_HTTP_BUILD_VERSION: v0.10.2
    -- ARROW_AWS_C_HTTP_BUILD_SHA256_CHECKSUM: 048d9d683459ade363fd7cc448c2b6329c78f67a2a0c0cb61c16de4634a2fc6b
    -- ARROW_AWS_C_IO_BUILD_VERSION: v0.19.1
    -- ARROW_AWS_C_IO_BUILD_SHA256_CHECKSUM: f2fea0c066924f7fe3c2b1c7b2fa9be640f5b16a6514854226330e63a1faacd0
    -- ARROW_AWS_C_MQTT_BUILD_VERSION: v0.13.1
    -- ARROW_AWS_C_MQTT_BUILD_SHA256_CHECKSUM: c54d02c1e46f55bae8d5e6f9c4b0d78d84c1c9d9ac16ba8d78c3361edcd8b5bb
    -- ARROW_AWS_C_S3_BUILD_VERSION: v0.8.1
    -- ARROW_AWS_C_S3_BUILD_SHA256_CHECKSUM: c8b09780691d2b94e50d101c68f01fa2d1c3debb0ff3aed313d93f0d3c9af663
    -- ARROW_AWS_C_SDKUTILS_BUILD_VERSION: v0.2.4
    -- ARROW_AWS_C_SDKUTILS_BUILD_SHA256_CHECKSUM: 493cbed4fa57e0d4622fcff044e11305eb4fc12445f32c8861025597939175fc
    -- ARROW_AWS_CHECKSUMS_BUILD_VERSION: v0.2.7
    -- ARROW_AWS_CHECKSUMS_BUILD_SHA256_CHECKSUM: 178e8398d98111f29150f7813a70c20ad97ab30be0de02525440355fe84ccb1d
    -- ARROW_AWS_CRT_CPP_BUILD_VERSION: v0.32.8
    -- ARROW_AWS_CRT_CPP_BUILD_SHA256_CHECKSUM: db44260452a0296341fb8e7b987e4c328f08f7829b9f1c740fed9c963e081e93
    -- ARROW_AWS_LC_BUILD_VERSION: v1.52.1
    -- ARROW_AWS_LC_BUILD_SHA256_CHECKSUM: fe552e3c3522f73afc3c30011745c431c633f7b4e25dcd7b38325f194a7b3b75
    -- ARROW_AWSSDK_BUILD_VERSION: 1.11.587
    -- ARROW_AWSSDK_BUILD_SHA256_CHECKSUM: b9944ba9905a68d6e53abb4f36ab2b3bd18ac88d8571647bb9f2b8026b76f8cd
    -- ARROW_AZURE_SDK_BUILD_VERSION: azure-identity_1.9.0
    -- ARROW_AZURE_SDK_BUILD_SHA256_CHECKSUM: 97065bfc971ac8df450853ce805f820f52b59457bd7556510186a1569502e4a1
    -- ARROW_BOOST_BUILD_VERSION: 1.88.0
    -- ARROW_BOOST_BUILD_SHA256_CHECKSUM: dcea50f40ba1ecfc448fdf886c0165cf3e525fef2c9e3e080b9804e8117b9694
    -- ARROW_BROTLI_BUILD_VERSION: v1.0.9
    -- ARROW_BROTLI_BUILD_SHA256_CHECKSUM: f9e8d81d0405ba66d181529af42a3354f838c939095ff99930da6aa9cdf6fe46
    -- ARROW_BZIP2_BUILD_VERSION: 1.0.8
    -- ARROW_BZIP2_BUILD_SHA256_CHECKSUM: ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269
    -- ARROW_CARES_BUILD_VERSION: 1.17.2
    -- ARROW_CARES_BUILD_SHA256_CHECKSUM: 4803c844ce20ce510ef0eb83f8ea41fa24ecaae9d280c468c582d2bb25b3913d
    -- ARROW_CRC32C_BUILD_VERSION: 1.1.2
    -- ARROW_CRC32C_BUILD_SHA256_CHECKSUM: ac07840513072b7fcebda6e821068aa04889018f24e10e46181068fb214d7e56
    -- ARROW_GBENCHMARK_BUILD_VERSION: v1.8.3
    -- ARROW_GBENCHMARK_BUILD_SHA256_CHECKSUM: 6bc180a57d23d4d9515519f92b0c83d61b05b5bab188961f36ac7b06b0d9e9ce
    -- ARROW_GFLAGS_BUILD_VERSION: v2.2.2
    -- ARROW_GFLAGS_BUILD_SHA256_CHECKSUM: 34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf
    -- ARROW_GLOG_BUILD_VERSION: v0.5.0
    -- ARROW_GLOG_BUILD_SHA256_CHECKSUM: eede71f28371bf39aa69b45de23b329d37214016e2055269b3b5e7cfd40b59f5
    -- ARROW_GOOGLE_CLOUD_CPP_BUILD_VERSION: v2.22.0
    -- ARROW_GOOGLE_CLOUD_CPP_BUILD_SHA256_CHECKSUM: 0c68782e57959c82e0c81def805c01460a042c1aae0c2feee905acaa2a2dc9bf
    -- ARROW_GRPC_BUILD_VERSION: v1.46.3
    -- ARROW_GRPC_BUILD_SHA256_CHECKSUM: d6cbf22cb5007af71b61c6be316a79397469c58c82a942552a62e708bce60964
    -- ARROW_GTEST_BUILD_VERSION: 1.16.0
    -- ARROW_GTEST_BUILD_SHA256_CHECKSUM: 78c676fc63881529bf97bf9d45948d905a66833fbfa5318ea2cd7478cb98f399
    -- ARROW_JEMALLOC_BUILD_VERSION: 5.3.0
    -- ARROW_JEMALLOC_BUILD_SHA256_CHECKSUM: 2db82d1e7119df3e71b7640219b6dfe84789bc0537983c3b7ac4f7189aecfeaa
    -- ARROW_LZ4_BUILD_VERSION: v1.10.0
    -- ARROW_LZ4_BUILD_SHA256_CHECKSUM: 537512904744b35e232912055ccf8ec66d768639ff3abe5788d90d792ec5f48b
    -- ARROW_MIMALLOC_BUILD_VERSION: v3.1.5
    -- ARROW_MIMALLOC_BUILD_SHA256_CHECKSUM: 1c6949032069d5ebea438ec5cedd602d06f40a92ddf0f0d9dcff0993e5f6635c
    -- ARROW_NLOHMANN_JSON_BUILD_VERSION: v3.12.0
    -- ARROW_NLOHMANN_JSON_BUILD_SHA256_CHECKSUM: 4b92eb0c06d10683f7447ce9406cb97cd4b453be18d7279320f7b2f025c10187
    -- ARROW_OPENTELEMETRY_BUILD_VERSION: v1.21.0
    -- ARROW_OPENTELEMETRY_BUILD_SHA256_CHECKSUM: 98e5546f577a11b52a57faed1f4cc60d8c1daa44760eba393f43eab5a8ec46a2
    -- ARROW_OPENTELEMETRY_PROTO_BUILD_VERSION: v1.7.0
    -- ARROW_OPENTELEMETRY_PROTO_BUILD_SHA256_CHECKSUM: 11330d850f5e24d34c4246bc8cb21fcd311e7565d219195713455a576bb11bed
    -- ARROW_ORC_BUILD_VERSION: 2.2.0
    -- ARROW_ORC_BUILD_SHA256_CHECKSUM: b15aca45a7e73ffbd1bbc36a78cd1422d41f07721092a25f43448e6e16f4763b
    -- ARROW_PROTOBUF_BUILD_VERSION: v21.3
    -- ARROW_PROTOBUF_BUILD_SHA256_CHECKSUM: 2f723218f6cb709ae4cdc4fb5ed56a5951fc5d466f0128ce4c946b8c78c8c49f
    -- ARROW_RAPIDJSON_BUILD_VERSION: 232389d4f1012dddec4ef84861face2d2ba85709
    -- ARROW_RAPIDJSON_BUILD_SHA256_CHECKSUM: b9290a9a6d444c8e049bd589ab804e0ccf2b05dc5984a19ed5ae75d090064806
    -- ARROW_RE2_BUILD_VERSION: 2022-06-01
    -- ARROW_RE2_BUILD_SHA256_CHECKSUM: f89c61410a072e5cbcf8c27e3a778da7d6fd2f2b5b1445cd4f4508bee946ab0f
    -- ARROW_SNAPPY_BUILD_VERSION: 1.2.2
    -- ARROW_SNAPPY_BUILD_SHA256_CHECKSUM: 90f74bc1fbf78a6c56b3c4a082a05103b3a56bb17bca1a27e052ea11723292dc
    -- ARROW_SUBSTRAIT_BUILD_VERSION: v0.44.0
    -- ARROW_SUBSTRAIT_BUILD_SHA256_CHECKSUM: f989a862f694e7dbb695925ddb7c4ce06aa6c51aca945105c075139aed7e55a2
    -- ARROW_S2N_TLS_BUILD_VERSION: v1.5.23
    -- ARROW_S2N_TLS_BUILD_SHA256_CHECKSUM: 81961ea5ae9313c987edfa579306ad4500bedfbf10caf84d8a5dcfc42aaf591f
    -- ARROW_THRIFT_BUILD_VERSION: 0.22.0
    -- ARROW_THRIFT_BUILD_SHA256_CHECKSUM: 794a0e455787960d9f27ab92c38e34da27e8deeda7a5db0e59dc64a00df8a1e5
    -- ARROW_UTF8PROC_BUILD_VERSION: v2.10.0
    -- ARROW_UTF8PROC_BUILD_SHA256_CHECKSUM: 6f4f1b639daa6dca9f80bc5db1233e9cbaa31a67790887106160b33ef743f136
    -- ARROW_XSIMD_BUILD_VERSION: 13.0.0
    -- ARROW_XSIMD_BUILD_SHA256_CHECKSUM: 8bdbbad0c3e7afa38d88d0d484d70a1671a1d8aefff03f4223ab2eb6a41110a3
    -- ARROW_ZLIB_BUILD_VERSION: 1.3.1
    -- ARROW_ZLIB_BUILD_SHA256_CHECKSUM: 9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23
    -- ARROW_ZSTD_BUILD_VERSION: 1.5.7
    -- ARROW_ZSTD_BUILD_SHA256_CHECKSUM: eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3
    -- Performing Test CMAKE_HAVE_LIBC_PTHREAD
    -- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
    -- Check if compiler accepts -pthread
    -- Check if compiler accepts -pthread - no
    -- Looking for pthread_create in pthreads
    -- Looking for pthread_create in pthreads - not found
    -- Looking for pthread_create in pthread
    -- Looking for pthread_create in pthread - not found
    CMake Error at /home/coworking/.conan2/p/cmake83736d917166f/p/share/cmake-3.31/Modules/FindPackageHandleStandardArgs.cmake:233 (message):
      Could NOT find Threads (missing: Threads_FOUND)
    Call Stack (most recent call first):
      /home/coworking/.conan2/p/cmake83736d917166f/p/share/cmake-3.31/Modules/FindPackageHandleStandardArgs.cmake:603 (_FPHSA_FAILURE_MESSAGE)
      /home/coworking/.conan2/p/cmake83736d917166f/p/share/cmake-3.31/Modules/FindThreads.cmake:226 (FIND_PACKAGE_HANDLE_STANDARD_ARGS)
      cmake_modules/ThirdpartyToolchain.cmake:1051 (find_package)
      CMakeLists.txt:523 (include)


    -- Configuring incomplete, errors occurred!

    arrow/22.0.0: ERROR: 
    Package '47bfb942dcd8247dce2a00ff6acd84c2b9f5bec6' build failed
    arrow/22.0.0: WARN: Build folder /home/coworking/.conan2/p/b/arrow25f5755664dfb/b/build/Release

