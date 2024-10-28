from conan import ConanFile
from conan.tools.cmake import CMakeDeps


class SiloRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [    
        "spdlog/1.14.1",
    ]

    
    default_options = {
   #     "fmt/*:header_only": True,
        "spdlog/*:header_only": True
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.set_property("spdlog", "cmake_find_mode", "both")
        deps.generate()

