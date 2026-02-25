#!/usr/bin/env python

import contextlib
import os
import os.path
from os.path import join as pjoin
import sys
import sysconfig

from setuptools import setup, Extension, Distribution
from Cython.Distutils import build_ext as _build_ext
import Cython

if Cython.__version__ < '3':
    raise Exception(
        'Please update your Cython version. Supported Cython >= 3')

# --- Utility Functions ---

ext_suffix = sysconfig.get_config_var('EXT_SUFFIX')

@contextlib.contextmanager
def changed_dir(dirname):
    oldcwd = os.getcwd()
    os.chdir(dirname)
    try:
        yield
    finally:
        os.chdir(oldcwd)

# --- Custom build_ext Command ---

class build_ext(_build_ext):
    _found_modules = []

    # Define custom options needed for CMake
    user_options = ([
        ('build-type=', None, 'build type (debug or release), default release'),
    ] + _build_ext.user_options)

    def initialize_options(self):
        _build_ext.initialize_options(self)
        # Default to Release for Python bindings (avoids ASan issues)
        self.build_type = "Release"

    def build_extensions(self):
        # Remove the dummy extension before building
        self.extensions = [ext for ext in self.extensions if ext.name != '__dummy__']
        _build_ext.build_extensions(self)

    def run(self):
        # 1. Run CMake build/install
        self._run_cmake()

        # 2. Let the parent class handle the rest
        _build_ext.run(self)

    def _run_cmake(self):
        """Run CMake to configure, build, and install the C++ targets."""
        # The directory containing this setup.py
        source = os.path.dirname(os.path.abspath(__file__))

        # Get setuptools build directories
        build_cmd = self.get_finalized_command('build')
        saved_cwd = os.getcwd()
        build_lib = pjoin(saved_cwd, build_cmd.build_lib)

        # Install directly to setuptools' build_lib directory
        # This way setuptools finds the files without manual copying
        install_prefix = pjoin(build_lib, "silodb")

        # Configuration name (e.g., Debug, Release)
        config_name = self.build_type.capitalize()

        # Reuse the existing cmake build directory (populated by `make dependencies`)
        # so that silolib is not recompiled for each Python version
        build_dir = pjoin(source, "build", config_name)

        if not os.path.isdir(build_dir):
            self.mkpath(build_dir)

        # Change to the build_dir directory
        with changed_dir(build_dir):
            # --- CONFIGURE ---
            cmake_options = [
                f'-DCMAKE_INSTALL_PREFIX={install_prefix}',
                f'-DPython3_EXECUTABLE={sys.executable}',
                f'-DCMAKE_BUILD_TYPE={config_name}',
                '-DBUILD_PYTHON_BINDINGS=ON',
                # var ignored on other OSs
                '-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0',
            ]

            print(f"-- Running cmake to configure project in {build_dir}")
            print(f"-- Will install to: {install_prefix}")
            self.spawn(['cmake'] + cmake_options + [source])

            # --- BUILD AND INSTALL ---
            print("-- Running cmake --build")

            build_tool_args = []
            if sys.platform != 'win32':
                build_tool_args.append('--')
                parallel = str(os.cpu_count() or 1)
                build_tool_args.append(f'-j{parallel}')

            # Build all targets
            self.spawn(['cmake', '--build', '.', '--config', config_name] + build_tool_args)

            # Install to setuptools' build directory
            print(f"-- Running cmake --build --target install")
            self.spawn(['cmake', '--build', '.', '--config', config_name, '--target', 'install'] + build_tool_args)

            print(f"-- CMake install finished. Files staged in: {install_prefix}")

        # Discover which modules were actually built
        self._found_modules = []
        module_names = ['database']
        for name in module_names:
            built_path = pjoin(install_prefix, name + ext_suffix)
            if os.path.exists(built_path):
                self._found_modules.append(name)
                print(f"-- Found built module: {name}")


    def _get_build_dir(self):
        """Get the package directory from build_py command"""
        build_py = self.get_finalized_command('build_py')
        return build_py.get_package_dir('silodb')

    def get_outputs(self):
        """Returns the list of built extension files (.so/.pyd)"""
        # Return paths to the built modules in setuptools' build directory
        return [pjoin(self._get_build_dir(), name + ext_suffix)
                for name in getattr(self, '_found_modules', [])]

# --- Distribution Setup ---

class BinaryDistribution(Distribution):
    """Custom distribution class to signal that this package has extension modules."""
    def has_ext_modules(foo):
        return True

setup(
    name="silodb",
    version="0.1.0",
    packages=["silodb"],
    package_dir={"silodb": "python/silodb"},
    
    package_data={
        "silodb": [
            "*.so", "*.pyd", # Compiled extensions
            "*.pxd", "*.pyx", # Cython source headers
            "libsilolib.so" # Core C++ library
        ],
    },
    
    distclass=BinaryDistribution,
    # Dummy extension is mandatory to trigger the 'build_ext' command
    ext_modules=[Extension('__dummy__', sources=[])],
    cmdclass={
        'build_ext': build_ext
    },
    
    python_requires=">=3.8",
    install_requires=[
        "Cython>=3.0",
        "pyarrow",
        "pyroaring",
    ],
    zip_safe=False,
)
