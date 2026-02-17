#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess


def clean_build_folder(build_folder: str):
    if os.path.exists(build_folder):
        shutil.rmtree(build_folder)


def run_cmd(context: str, cmd: str):
    print("----------------------------------")
    print(cmd)
    print("----------------------------------")

    if subprocess.call(cmd, shell=True) != 0:
        raise Exception(f"{context} command failed.")


def main(args):
    cmake_options = []
    conan_options = []
    if args.build_with_clang_tidy:
        cmake_options.append("-D BUILD_WITH_CLANG_TIDY=ON")

    if args.release:
        build_folder = "build/Release"
        cmake_options.append("-D CMAKE_BUILD_TYPE=Release")
        conan_options.append("--output-folder=build/Release/generators")
    else:
        build_folder = "build/Debug"
        cmake_options.append("-D CMAKE_BUILD_TYPE=Debug")
        # We still want to have our dependencies in Release, but the & tells conan we will build the consumer as Debug
        conan_options.append("--settings '&:build_type=Debug'")
        conan_options.append("--output-folder=build/Debug/generators")

    # TODO(#986) remove this when arrow is fixed
    conan_options.append("--settings 'arrow/*:compiler.cppstd=20'")

    if args.clean:
        print("----------------------------------")
        print(f"cleaning build directory {build_folder}")
        print("----------------------------------")
        clean_build_folder(build_folder)

    os.makedirs(build_folder, exist_ok=True)

    run_cmd("Conan install",
            "conan install . --update --build=missing --profile ./conanprofile --profile:build ./conanprofile " + " ".join(
                conan_options))

    run_cmd("CMake", "cmake -G Ninja " + " ".join(cmake_options) + f" -B {build_folder}")

    run_cmd("CMake build", f"cmake --build {build_folder} --parallel {args.parallel}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--clean", action="store_true", help="Clean build directory before building")
    parser.add_argument("--release", action="store_true", help="Trigger RELEASE build")
    parser.add_argument("--build_with_clang_tidy", action="store_true", help="Build with clang-tidy")
    parser.add_argument("--parallel", type=int, default=16, help="Number of parallel jobs")

    args_parsed = parser.parse_args()
    main(args_parsed)
