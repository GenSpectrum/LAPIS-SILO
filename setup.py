# setup.py
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import os
import sys
import subprocess
import shutil
from pathlib import Path

class CMakeBuildExt(build_ext):
    """Custom build_ext that runs CMake to build extensions"""

    def run(self):
        # Run CMake to build everything
        self._run_cmake()
        # Copy .so files to package directory
        self._copy_extensions()
        # Don't call parent run() - CMake already built the extensions

    def _run_cmake(self):
        """Run CMake to configure and build the project"""
        source_dir = Path(__file__).parent.absolute()
        build_dir = source_dir / "build" / "Debug"

        # Create build directory if it doesn't exist
        build_dir.mkdir(parents=True, exist_ok=True)

        # Check if CMake has already been configured
        cmake_cache = build_dir / "CMakeCache.txt"
        if not cmake_cache.exists():
            print("-- Running cmake to configure project")
            # Configure with CMake
            cmake_args = [
                'cmake',
                f'-S{source_dir}',
                f'-B{build_dir}',
                '-DBUILD_PYTHON_BINDINGS=ON',
            ]
            subprocess.check_call(cmake_args)
            print("-- Finished cmake configuration")
        else:
            print("-- CMake already configured, skipping configuration")

        # Build the project
        print("-- Running cmake --build")
        build_args = [
            'cmake',
            '--build', str(build_dir),
            '--target', 'silolib',
            '-j', str(os.cpu_count() or 1)
        ]
        subprocess.check_call(build_args)
        print("-- Finished cmake --build for silolib")

        # Build Python modules
        print("-- Building Python extension modules")
        for module in ['column_type', 'column_identifier', 'database']:
            module_args = [
                'cmake',
                '--build', str(build_dir),
                '--target', module
            ]
            try:
                subprocess.check_call(module_args)
                print(f"-- Built {module} successfully")
            except subprocess.CalledProcessError as e:
                print(f"-- Warning: Failed to build {module}: {e}")
        print("-- Finished building Python extension modules")

    def _copy_extensions(self):
        """Copy built .so files to the build lib directory for installation"""
        source_dir = Path(__file__).parent.absolute()
        cmake_build_dir = source_dir / "build" / "Debug" / "python" / "pysilo"

        # Get the build lib directory from setuptools
        build_lib = Path(self.build_lib) / "pysilo"
        build_lib.mkdir(parents=True, exist_ok=True)

        print(f"-- Copying extension modules to {build_lib}")
        for module in ['column_type', 'column_identifier', 'database']:
            for ext in ['.so', '.pyd']:
                so_file = cmake_build_dir / f"{module}{ext}"
                if so_file.exists():
                    dest_file = build_lib / f"{module}{ext}"
                    print(f"-- Copying {so_file} to {dest_file}")
                    shutil.copy2(so_file, dest_file)

        # Also copy libsilolib.so to the package directory
        silolib = source_dir / "build" / "Debug" / "libsilolib.so"
        if silolib.exists():
            dest_lib = build_lib / "libsilolib.so"
            print(f"-- Copying {silolib} to {dest_lib}")
            shutil.copy2(silolib, dest_lib)

    def get_outputs(self):
        """Return list of built extension files"""
        # Return the .so files that CMake built
        build_dir = Path(__file__).parent / "build" / "Debug" / "python" / "pysilo"
        outputs = []
        if build_dir.exists():
            for module in ['column_type', 'column_identifier', 'database']:
                # Find the .so file (or .pyd on Windows)
                for ext in ['.so', '.pyd']:
                    so_file = build_dir / f"{module}{ext}"
                    if so_file.exists():
                        outputs.append(str(so_file))
        return outputs

setup(
    name="pysilo",
    version="0.1.0",
    packages=["pysilo"],
    package_dir={"pysilo": "python/pysilo"},
    package_data={
        "pysilo": ["*.so", "*.pyd"],  # Include compiled extension modules
    },
    # Dummy extension to trigger build_ext
    ext_modules=[Extension("pysilo.__dummy__", sources=[])],
    cmdclass={
        'build_ext': CMakeBuildExt,
    },
    python_requires=">=3.8",
    install_requires=[
        "Cython>=3.0",
    ],
    zip_safe=False,
)
