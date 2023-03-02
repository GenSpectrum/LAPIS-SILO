# LAPIS-SILO
Sequence Indexing engine for Large Order of genomic data

# License
Original genome indexing logic with roaring bitmaps by Prof. Neumann: https://db.in.tum.de/~neumann/gi/


# Building

## With Conan
for local development (tested on Ubuntu 22.04.2 LTS):
- requires: python3, pip3

Install conan version 1.59.0. We use this version because TBB is not yet ported to version 2.0.   
```shell
pip3 install conan==1.59.0
```
Set path for pip (if not already set)
```shell
export PATH="$HOME/.local/bin:$PATH"
```
Get default profile (myProfile) of own system and write to `~/.conan/profiles/myProfile`
```shell
conan profile new myProfile --detect
```
Insert info `os`, `os_build`, `arch` and `arch_build` of myProfile into `conanprofile.example` and rename to `conanprofile`.

Build silo in `./build`. This build will load and build the required libraries to `~/.conan/data/` (can not be set by hand). 
```shell
./build_with_conan
```
Run silo
```shell
./build/silo
```

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