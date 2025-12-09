#!/usr/bin/env python

import contextlib
import os
import os.path
from os.path import join as pjoin
import sys
import warnings

# Use distutils to ensure compatibility across Python versions for sysconfig
from distutils import sysconfig 

from setuptools import setup, Extension, Distribution
from Cython.Distutils import build_ext as _build_ext
import Cython

# Check Cython version requirement
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

    # Define custom options needed for CMake
    user_options = ([
        ('build-type=', None, 'build type (debug or release), default release'),
    ] + _build_ext.user_options)

    def initialize_options(self):
        _build_ext.initialize_options(self)

    def run(self):
        # 1. Run CMake to configure, build, and install targets
        self._run_cmake()
        
        # 2. Let the parent class handle copying the installed files
        # from the temporary install prefix to the final build/lib folder.
        _build_ext.run(self)

    def _run_cmake(self):
        """Run CMake to configure, build, and install the C++ targets."""
        # The directory containing this setup.py
        source = os.path.dirname(os.path.abspath(__file__))

        # Get setuptools build directories
        build_cmd = self.get_finalized_command('build')
        saved_cwd = os.getcwd()
        build_temp = pjoin(saved_cwd, build_cmd.build_temp)

        if not os.path.isdir(build_temp):
            self.mkpath(build_temp)
        
        # Define the temporary install location for CMake (crucial)
        install_prefix = pjoin(build_temp, "install") 

        # Change to the build directory
        with changed_dir(build_temp):
            
            # --- CONFIGURE ---
            cmake_options = [
                f'-DCMAKE_INSTALL_PREFIX={install_prefix}',
                f'-DPYTHON_EXECUTABLE={sys.executable}',
                # Your project-specific flag
                '-DBUILD_PYTHON_BINDINGS=ON', 
                f'-DCMAKE_BUILD_TYPE={self.build_type.upper()}'
            ]
            
            print("-- Running cmake to configure project")
            self.spawn(['cmake'] + cmake_options + [source])
            
            # --- BUILD AND INSTALL (Consolidated) ---
            print("-- Running cmake --build and install targets")
            
            # Build tool arguments (e.g., -j for parallel building)
            build_tool_args = []
            if sys.platform != 'win32':
                build_tool_args.append('--')
                parallel = str(os.cpu_count() or 1)
                build_tool_args.append(f'-j{parallel}')

            # Use the 'install' target to build and stage ALL targets (modules, libsilolib)
            self.spawn(['cmake', 
                        '--build', '.', 
                        '--config', self.build_type, 
                        '--target', 'install'] + build_tool_args)
            
            print(f"-- CMake install finished. Files staged in: {install_prefix}")


    def get_outputs(self):
        """Returns the list of built extension files (.so/.pyd)"""
        # We rely on the install step to place files in the package directory
        build_py = self.get_finalized_command('build_py')
        build_dir = build_py.get_package_dir('pysilo')
        
        # Manually list expected module names
        module_names = ['column_type', 'column_identifier', 'database']
        
        outputs = []
        for name in module_names:
            # Construct the expected final path in the build directory
            filename = name + ext_suffix
            outputs.append(pjoin(build_dir, filename))
            
        # Add the core library (libsilolib.so) if it's installed alongside
        outputs.append(pjoin(build_dir, "libsilolib" + ext_suffix))
        
        return outputs

    # The other methods like get_names, get_ext_generated_cpp_source, etc., 
    # are removed as they are specific to Arrow's complex build tracking.

# --- Distribution Setup ---

class BinaryDistribution(Distribution):
    """Custom distribution class to signal that this package has extension modules."""
    def has_ext_modules(foo):
        return True


setup(
    name="pysilo",
    version="0.1.0",
    packages=["pysilo"],
    package_dir={"pysilo": "python/pysilo"},
    
    # Define package data so setuptools knows to include the compiled extensions
    # and Cython headers in the final wheel/sdist.
    package_data={
        "pysilo": [
            "*.so", "*.pyd", # Compiled extensions
            "*.pxd", "*.pyx", # Cython source headers (CRUCIAL FIX)
            "libsilolib.so" # Core C++ library (MUST be installed by CMake)
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
    ],
    zip_safe=False,
)