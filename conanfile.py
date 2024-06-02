from conan import ConanFile
from conan.tools.cmake import CMakeDeps


class SiloRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    # Update regularly
    requires = [
        "boost/1.83.0", # https://conan.io/center/recipes?value=boosta #FIXME: Serialization error with 1.84.0
        "duckdb/0.8.1", # https://conan.io/center/recipes?value=duckdb #FIXME: test files only compatible with 0.8, need to EXPORT DATABASE in v0.8, then IMPORT DATABASE on the current version of DuckDB
        "gtest/1.14.0", # https://conan.io/center/recipes?value=gtest
        "hwloc/2.9.3", # https://conan.io/center/recipes?value=hwloc # BLOCKED: Pinned by onetbb <= 2021.12.0
        "nlohmann_json/3.11.3", # https://conan.io/center/recipes?value=nlohmann_json
        "onetbb/2021.12.0", # https://conan.io/center/recipes?value=onetbb
        "poco/1.13.3", # https://conan.io/center/recipes?value=poco
        "roaring/4.0.0", # https://conan.io/center/recipes?value=roaring
        "spdlog/1.11.0", # https://conan.io/center/recipes?value=spdlog #FIXME: 1.12.0 fails with `error: implicit instantiation of undefined template` #TODO: holding back upgrade to 1.14.1
        "yaml-cpp/0.8.0", # https://conan.io/center/recipes?value=yaml-cpp
        "zstd/1.5.5", # https://conan.io/center/recipes?value=zstd #BLOCKED: Pinned by boost <=1.85.0
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
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.set_property("boost", "cmake_find_mode", "both")
        deps.set_property("duckdb", "cmake_find_mode", "both")
        deps.set_property("fmt", "cmake_find_mode", "both")
        deps.set_property("gtest", "cmake_find_mode", "both")
        deps.set_property("hwloc", "cmake_find_mode", "both")
        deps.set_property("nlohmann_json", "cmake_find_mode", "both")
        deps.set_property("onetbb", "cmake_find_mode", "both")
        deps.set_property("pcre2", "cmake_find_mode", "both")
        deps.set_property("poco", "cmake_find_mode", "both")
        deps.set_property("roaring", "cmake_find_mode", "both")
        deps.set_property("spdlog", "cmake_find_mode", "both")
        deps.set_property("yaml-cpp", "cmake_find_mode", "both")
        deps.set_property("zstd", "cmake_find_mode", "both")
        deps.generate()
