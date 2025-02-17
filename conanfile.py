from conan import ConanFile
from conan.tools.cmake import CMakeDeps


class SiloRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [
        "asio/1.29.0",
        "boost/1.82.0",
        "crowcpp-crow/1.2.0",
        "duckdb/1.0.0",
        "onetbb/2021.10.0",
        "nlohmann_json/3.11.2",
        "gtest/cci.20210126",
        "roaring/1.0.0",
        "spdlog/1.14.1",
        "yaml-cpp/0.7.0",
        "zstd/1.5.5",
        "re2/20240702",
        "abseil/20240116.1",
    ]

    default_options = {
        "yaml-cpp/*:shared": False,

        "zstd/*:shared": False,

        "duckdb/*:shared": False,
        "duckdb/*:with_json": True,
        "duckdb/*:with_parquet": True,

        "roaring/*:shared": False,

        "gtest/*:no_main": True,

        "boost/*:lzma": True,
        "boost/*:zstd": True,
        "boost/*:shared": False,

        "hwloc/*:shared": False,

        "boost/*:without_iostreams": False,
        "boost/*:without_serialization": False,
        "boost/*:without_system": False,
        "boost/*:without_random": False,
        "boost/*:without_regex": False,

        "boost/*:without_atomic": True,
        "boost/*:without_chrono": True,
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
        "boost/*:without_stacktrace": True,
        "boost/*:without_test": True,
        "boost/*:without_thread": True,
        "boost/*:without_timer": True,
        "boost/*:without_type_erasure": True,
        "boost/*:without_wave": True,

        "crowcpp-crow/*:shared": False,

        "re2/*:shared": False,

        "abseil/*:shared": False,
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.set_property("boost", "cmake_find_mode", "both")
        deps.set_property("duckdb", "cmake_find_mode", "both")
        deps.set_property("crowcpp-crow", "cmake_find_mode", "both")
        deps.set_property("gtest", "cmake_find_mode", "both")
        deps.set_property("hwloc", "cmake_find_mode", "both")
        deps.set_property("nlohmann_json", "cmake_find_mode", "both")
        deps.set_property("onetbb", "cmake_find_mode", "both")
        deps.set_property("pcre2", "cmake_find_mode", "both")
        deps.set_property("roaring", "cmake_find_mode", "both")
        deps.set_property("spdlog", "cmake_find_mode", "both")
        deps.set_property("yaml-cpp", "cmake_find_mode", "both")
        deps.set_property("zstd", "cmake_find_mode", "both")
        deps.set_property("re2", "cmake_find_mode", "both")
        deps.set_property("abseil", "cmake_find_mode", "both")
        deps.generate()
