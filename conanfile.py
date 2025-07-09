from conan import ConanFile
from conan.tools.cmake import CMakeDeps


class SiloRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [
        "arrow/19.0.1",
        "boost/1.85.0",
        "poco/1.13.3",
        "onetbb/2022.0.0",
        "nlohmann_json/3.12.0",
        "gtest/1.16.0",
        "re2/20240702",
        "roaring/4.2.1",
        "simdjson/3.12.3",
        "spdlog/1.15.1",
        "yaml-cpp/0.8.0",
        "zstd/1.5.7",
    ]

    default_options = {
        "abseil/*:shared": False,

        "arrow/*:compute": True,
        "arrow/*:acero": True,

        "boost/*:lzma": True,
        "boost/*:zstd": True,
        "boost/*:shared": False,

        "boost/*:without_iostreams": False,
        "boost/*:without_serialization": False,
        "boost/*:without_container": False,
        "boost/*:without_system": False,
        "boost/*:without_random": False,
        "boost/*:without_regex": False,
        "boost/*:without_atomic": True,
        "boost/*:without_chrono": True,
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

        "gtest/*:no_main": True,

        "hwloc/*:shared": False,

        "poco/*:shared": False,
        "poco/*:enable_json": True,
        "poco/*:enable_net": True,
        "poco/*:enable_util": True,

        "poco/*:enable_crypto": False,
        "poco/*:enable_activerecord": False,
        "poco/*:enable_active_record": False,
        "poco/*:enable_data": False,
        "poco/*:enable_data_mysql": False,
        "poco/*:enable_data_postgresql": False,
        "poco/*:enable_data_sqlite": False,
        "poco/*:enable_encodings": False,
        "poco/*:enable_jwt": False,
        "poco/*:enable_mongodb": False,
        "poco/*:enable_netssl": False,
        "poco/*:enable_redis": False,
        "poco/*:enable_xml": False,
        "poco/*:enable_zip": False,

        "re2/*:shared": False,

        "roaring/*:shared": False,

        "simdjson/*:shared": False,

        "spdlog/*:shared": False,

        "yaml-cpp/*:shared": False,

        "zstd/*:shared": False,
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.set_property("abseil", "cmake_find_mode", "both")
        deps.set_property("boost", "cmake_find_mode", "both")
        deps.set_property("gtest", "cmake_find_mode", "both")
        deps.set_property("hwloc", "cmake_find_mode", "both")
        deps.set_property("nlohmann_json", "cmake_find_mode", "both")
        deps.set_property("onetbb", "cmake_find_mode", "both")
        deps.set_property("pcre2", "cmake_find_mode", "both")
        deps.set_property("poco", "cmake_find_mode", "both")
        deps.set_property("re2", "cmake_find_mode", "both")
        deps.set_property("roaring", "cmake_find_mode", "both")
        deps.set_property("spdlog", "cmake_find_mode", "both")
        deps.set_property("simdjson", "cmake_find_mode", "both")
        deps.set_property("yaml-cpp", "cmake_find_mode", "both")
        deps.set_property("zstd", "cmake_find_mode", "both")
        deps.generate()
