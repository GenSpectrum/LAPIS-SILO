# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize
import os

# Define include directories
include_dirs = [
    "src",  # Your C++ headers
    "python/pysilo",  # Add this so Cython can find .pxd files
]

library_dirs = []
libraries = []

extra_compile_args = [
    "-std=c++17",
    "-O3",
]

extensions = [
    Extension(
        "pysilo.column_type",
        sources=["python/pysilo/column_type.pyx"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
        language="c++",
    ),
    Extension(
        "pysilo.column_identifier",
        sources=["python/pysilo/column_identifier.pyx"],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
        language="c++",
    ),
    Extension(
        "pysilo.database",
        sources=[
            "python/pysilo/database.pyx",
            # Add your C++ source files here
            # "src/database.cpp",
        ],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
        language="c++",
    ),
]

setup(
    name="pysilo",
    version="0.1.0",
    packages=["pysilo"],
    package_dir={"pysilo": "python/pysilo"},
    ext_modules=cythonize(
        extensions,
        compiler_directives={
            "language_level": "3",
            "embedsignature": True,
        },
        include_path=["python/pysilo"],  # Add this!
    ),
    python_requires=">=3.8",
    zip_safe=False,
)
