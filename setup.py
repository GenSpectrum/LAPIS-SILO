#!/usr/bin/env python

import contextlib
import os
import os.path
from os.path import join as pjoin
import sys
from distutils import sysconfig 

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
    
    # Define custom options needed for CMake
    user_options = ([
        ('build-type=', None, 'build type (debug or release), default release'),
    ] + _build_ext.user_options)

    def initialize_options(self):
        _build_ext.initialize_options(self)
        # Default to Release unless environment variable is set
        self.build_type = "Debug"

    def run(self):
        # 1. Run CMake build/install
        self._run_cmake()
        
        # 2. Let the parent class handle copying the installed files
        _build_ext.run(self)

    def _run_cmake(self):
        """Run CMake to configure, build, and install the C++ targets."""
        source = os.path.dirname(os.path.abspath(__file__))

        # Configuration name (e.g., Debug, Release)
        config_name = self.build_type.capitalize()
        
        # Calculate the desired build directory: <project_root>/build/Debug or /Release
        build_dir = pjoin(source, "build", config_name)
        
        if not os.path.isdir(build_dir):
            self.mkpath(build_dir)
        
        # Define the temporary install location for CMake. 
        # This directory must be inside the build directory for setuptools to find it.
        install_prefix = pjoin(build_dir, "install") 

        # Change to the build directory
        with changed_dir(build_dir):
            
            # --- CONFIGURE ---
            cmake_options = [
                # Use -S and -B to point to the source and the build directory
                # Since we are already inside the build_dir, -B is '.'
                f'-S{source}', 
                f'-B{build_dir}',
                
                f'-DCMAKE_INSTALL_PREFIX={install_prefix}',
                f'-DPYTHON_EXECUTABLE={sys.executable}',
                
                # Pass the configuration type to CMake
                f'-DCMAKE_BUILD_TYPE={config_name}',
                
                # Your project-specific flag
                '-DBUILD_PYTHON_BINDINGS=ON', 
            ]
            
            print(f"-- Running cmake to configure project in {build_dir}")
            # Use 'cmake' command instead of self.spawn for clarity of initial configuration
            os.environ['CMAKE_COMMAND'] = 'cmake'
            self.spawn(['cmake'] + cmake_options)
            
            # --- BUILD AND INSTALL (Consolidated) ---
            print("-- Running cmake --build and install targets")
            
            build_tool_args = []
            if sys.platform != 'win32':
                build_tool_args.append('--')
                parallel = str(os.cpu_count() or 1)
                build_tool_args.append(f'-j{parallel}')

            # Build and stage ALL targets (modules, libsilolib)
            self.spawn(['cmake', 
                        '--build', '.', 
                        '--config', config_name, # Pass the config name again
                        '--target', 'install'] + build_tool_args)
            
            print(f"-- CMake install finished. Files staged in: {install_prefix}")


    def get_outputs(self):
        """Returns the list of built extension files (.so/.pyd)"""
        
        # 1. Determine the build directory where files were staged
        config_name = self.build_type.capitalize()
        build_dir_root = pjoin(os.path.dirname(os.path.abspath(__file__)), "build", config_name)
        
        # 2. Get the package directory where files were installed inside the staging area
        # This should match the DESTINATION used in your CMakeLists.txt (e.g., python/pysilo)
        installed_package_dir = pjoin(build_dir_root, "install", "python", "pysilo")
        
        outputs = []
        # Manually list expected module names
        module_names = ['column_type', 'column_identifier', 'database']
        
        for name in module_names:
            filename = name + ext_suffix
            # Crucially, we return the path from the final build/lib folder, 
            # not the temp staging folder.
            outputs.append(pjoin(self.get_finalized_command('build_py').get_package_dir('pysilo'), filename))
        
        # Add the core library 
        outputs.append(pjoin(self.get_finalized_command('build_py').get_package_dir('pysilo'), "libsilolib" + ext_suffix))
        
        return outputs

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
    
    package_data={
        "pysilo": [
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
    ],
    zip_safe=False,
)
