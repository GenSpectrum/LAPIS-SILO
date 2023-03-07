# LAPIS-SILO
Sequence Indexing engine for Large Order of genomic data

# License
Original genome indexing logic with roaring bitmaps by Prof. Neumann: https://db.in.tum.de/~neumann/gi/


# Building

## With Conan
We use Conan to install dependencies for local development. See Dockerfile for how to set up Conan and its requirements.
This has been tested on Ubuntu 22.04 and is not guaranteed to work on other systems.

The Conan profile (myProfile) on your system might differ: Create a new profile `~/.conan/profiles/myProfile`
```shell
conan profile new myProfile --detect
```
Insert info `os`, `os_build`, `arch` and `arch_build` of myProfile into `conanprofile.example` and rename to `conanprofile`.

Build silo in `./build`. This build will load and build the required libraries to `~/.conan/data/` (can not be set by hand). 
```shell
./build_with_conan
```
Executables are located in `build/` upon a successful build.

## With Docker:
(for CI and release)

Build docker container
```shell
docker build . --tag=silo
```
Run docker container
```shell
docker run -i silo
```

# Testing
For testing we use the framework [gtest](http://google.github.io/googletest/) and for mocking [gmock](http://google.github.io/googletest/gmock_cook_book.html). Tests are built using the same script as the production code: `./build_with_conan`.

We use the convention, that each tested source file has its own test file, ending with `*.test.cpp`. The test file is placed in the same folder as the source file. If the function under test is described in a header file, the test file is located in the corresponding source folder. 


To run all tests, run
```shell
build/silo_test
```