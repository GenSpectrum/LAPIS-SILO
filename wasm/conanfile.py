from conan import ConanFile
from conan.tools.cmake import CMakeDeps


class SiloWasmRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [
        "arrow/22.0.0",
        "boost/1.85.0",
        "fast_float/8.1.0",
        "nlohmann_json/3.12.0",
        "re2/20251105",
        "roaring/4.5.0",
        "simdjson/4.2.4",
        "simdutf/8.0.0",
        "spdlog/1.17.0",
        "yaml-cpp/0.8.0",
        "zstd/1.5.7",
    ]

    default_options = {
        "abseil/*:shared": False,

        "arrow/*:with_mimalloc": False,
        "arrow/*:compute": True,
        "arrow/*:acero": True,
        "arrow/*:filesystem_layer": False,
        "arrow/*:parquet": False,
        "arrow/*:with_thrift": False,
        "arrow/*:with_zlib": False,

        "boost/*:lzma": False,
        "boost/*:zstd": True,
        "boost/*:multithreading": False,
        "boost/*:shared": False,
        "boost/*:without_iostreams": False,
        "boost/*:without_serialization": False,
        "boost/*:without_system": False,
        "boost/*:without_atomic": True,
        "boost/*:without_charconv": True,
        "boost/*:without_chrono": True,
        "boost/*:without_cobalt": True,
        "boost/*:without_container": True,
        "boost/*:without_context": True,
        "boost/*:without_contract": True,
        "boost/*:without_coroutine": True,
        "boost/*:without_date_time": True,
        "boost/*:without_exception": True,
        "boost/*:without_fiber": True,
        "boost/*:without_filesystem": True,
        "boost/*:without_graph": True,
        "boost/*:without_graph_parallel": True,
        "boost/*:without_json": True,
        "boost/*:without_locale": True,
        "boost/*:without_log": True,
        "boost/*:without_math": True,
        "boost/*:without_mpi": True,
        "boost/*:without_nowide": True,
        "boost/*:without_program_options": True,
        "boost/*:without_python": True,
        "boost/*:without_random": False,
        "boost/*:without_regex": False,
        "boost/*:without_stacktrace": True,
        "boost/*:without_test": True,
        "boost/*:without_thread": True,
        "boost/*:without_timer": True,
        "boost/*:without_type_erasure": True,
        "boost/*:without_url": True,
        "boost/*:without_wave": True,

        "hwloc/*:shared": False,
        "re2/*:shared": False,
        "roaring/*:shared": False,
        "simdjson/*:shared": False,
        "simdutf/*:shared": False,
        "spdlog/*:shared": False,
        "yaml-cpp/*:shared": False,
        "zstd/*:shared": False,
    }

    def generate(self):
        deps = CMakeDeps(self)
        for package in [
            "abseil",
            "boost",
            "hwloc",
            "nlohmann_json",
            "pcre2",
            "re2",
            "roaring",
            "spdlog",
            "simdjson",
            "simdutf",
            "yaml-cpp",
            "zstd",
        ]:
            deps.set_property(package, "cmake_find_mode", "both")
        deps.generate()
