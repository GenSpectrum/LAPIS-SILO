# LAPIS-SILO

Sequence Indexing engine for Large Order of genomic data

# License

Original genome indexing logic with roaring bitmaps by Prof. Neumann: https://db.in.tum.de/~neumann/gi/

# Building

Use `./build_with_conan.py` to build SILO. `./build_with_conan.py --help` shows all available options.

We took the approach to scan directories for .cpp files to include instead of listing them manually in the
CMakeLists.txt. This has the advantage that we don't need to maintain a list of files in the CMakeLists.txt.

It has the disadvantage that after a successful build on local machines, CMake can't detect whether files were
added or deleted. This requires a clean build. You can either delete the `build/` directory manually, or you
execute `./build_with_conan.py --clean`.

Since in any approach, a developer has to remember to either trigger a clean build or to adapt the CMakeLists.txt, we
decided for the approach with less maintenance effort, since it will automatically work in GitHub Actions.

### Running SILO locally with CLion

`.run` contains run configurations for CLion that are ready to use.
They assume that you configured CMake in CLion to use `./build` as build directory.
CLion should be able to detect those files automatically.

## With Conan

We use Conan to install dependencies for local development. See Dockerfile for how to set up Conan and its requirements.
This has been tested on Ubuntu 22.04 and is not guaranteed to work on other systems.

The Conan profile (myProfile) on your system might differ: Create a new profile `~/.conan2/profiles/myProfile`

```shell
conan profile detect
```

Copy `conanprofile.example` to `conanprofile` and insert the values of `os`, `os_build`, `arch` and `arch_build` from
myProfile.

Build silo in `./build`. This build will load and build the required libraries to `~/.conan2/data/` (can not be set by
hand).

```shell
./build_with_conan.py
```

Executables are located in `build/` upon a successful build.

## With Docker:

(for CI and release)

Build docker container

```shell
docker build . --tag=silo
```

The Docker images are built in such a way that they can be used for both,
preprocessing and running the api, with minimal configuration.
The images contain default configuration so that a user only needs to mount data to the correct locations.

## Configuration Files

For SILO, there are three different configuration files:

- `DatabaseConfig` described in
  file [database_config.h](https://github.com/GenSpectrum/LAPIS-SILO/blob/main/include/config/database_config.h)
- `PreprocessingConfig` used when started with `--preprocessing` and described in
  file [preprocessing_config.h](https://github.com/GenSpectrum/LAPIS-SILO/blob/main/include/config/preprocessing_config.h)
- `RuntimeConfig` used when started with `--api` and described in
  file [runtime_config.h](https://github.com/GenSpectrum/LAPIS-SILO/blob/main/include/config/preprocessing_config.h)

The database config contains the schema of the database and is always required when preprocessing data. The database
config will be saved together with the output of the preprocessing and is therefore not required when starting SILO as
an API.
An example of a configuration file can be seen
in [testBaseData/exampleDataset/database_config.yaml](https://github.com/GenSpectrum/LAPIS-SILO/blob/main/testBaseData/exampleDataset/database_config.yaml).

By default, the config files are expected to be YAML files in the current working directory in
snake_case (`database_config.yaml`, `preprocessing_config.yaml`, `runtime_config.yaml`), but their location can be
overridden using the options `--databaseConfig=X`, `--preprocessingConfig=X`, and `--runtimeConfig=X`.

Preprocessing and Runtime configurations contain default values for all fields and are thus only optional. Their
parameters can also be provided as command-line arguments in snake_case and as environment variables prefixed with SILO_
in capital SNAKE_CASE. (e.g. SILO_INPUT_DIRECTORY).

The precendence is `CLI argument > Environment Variable > Configuration File > Default Value`

### Run The Preprocessing

The preprocessing acts as a program that takes an input directory that contains the to-be-processed data
and an output directory where the processed data will be stored.
Both need to be mounted to the container.
SILO also expects a database config and a preprocessing config that need to be mounted to the default locations.

```shell
docker run \
  -v your/input/directory:/preprocessing/input \
  -v your/preprocessing/output:/preprocessing/output \
  -v your/preprocessing_config.yaml:/app/preprocessing_config.yaml \
  -v your/database_config.yaml:/app/database_config.yaml \
  silo --preprocessing
```

Both config files can also be provided in custom locations:

```shell
silo --preprocessing --preprocessingConfig=./custom/preprocessing_config.yaml --databaseConfig=./custom/database_config.yaml
```

The Docker image contains a default preprocessing config that sets defaults specific for running SILO in Docker.
Apart from that, there are default values if neither user-provided nor default config specify fields.
The user-provided preprocessing config can be used to overwrite the default values. For a full reference, see

* [testBaseData/test_preprocessing_config_with_overridden_defaults.yaml](https://github.com/GenSpectrum/LAPIS-SILO/blob/main/testBaseData/test_preprocessing_config_with_overridden_defaults.yaml)

### Run docker container (api)

After preprocessing the data, the api can be started with the following command:

```shell
docker run 
  -p 8081:8081
  -v your/preprocessing/output:/data
  silo --api
```

The directory where SILO expects the preprocessing output can be overwritten via
`silo --api --dataDirectory=/custom/data/directory` or in a corresponding
configuration [configuration file](#configuration-files).

### Notes On Building The Image

Building Docker images locally relies on the local Docker cache.
Docker will cache layers, and it will cache the dependencies built by Conan via cache mounts.

However, cache mounts don't work in GitHub Actions (https://github.com/docker/build-push-action/issues/716),
so there we only rely on Docker's layer cache via Docker's gha cache backend.

# Testing

## Unit Tests

For testing, we use the framework [gtest](http://google.github.io/googletest/)
and [gmock](http://google.github.io/googletest/gmock_cook_book.html) for mocking. Tests are built using the same script
as the production code: `./build_with_conan`.

We use the convention, that each tested source file has its own test file, ending with `*.test.cpp`. The test file is
placed in the same folder as the source file. If the function under test is described in a header file, the test file is
located in the corresponding source folder.

To run all tests, run

```shell
build/silo_test
```

For linting we use clang-tidy. The config is stored in `.clang-tidy`.

When pushing to GitHub, a separate Docker image will be built, which runs the formatter. (This is a workaround, because
building with clang-tidy under alpine was not possible yet.)

## Functional End-To-End Tests

End-to-end tests are located in `/endToEndTests`. Those tests are used to verify the overall functionality of the SILO
queries. To execute the tests:

* have a running SILO instance with preprocessd data e.g. via
    * `SILO_IMAGE=ghcr.io/genspectrum/lapis-silo docker compose -f docker-compose-for-tests-preprocessing.yml up`
    * `SILO_IMAGE=ghcr.io/genspectrum/lapis-silo docker compose -f docker-compose-for-tests-api.yml up -d wait`
* `cd endToEndTests`
* `npm install`
* `SILO_URL=localhost:8081 npm run test`

# Logging

We use [spdlog](https://github.com/gabime/spdlog) for logging.
The log level can be controlled via the environment variable `SPDLOG_LEVEL`:

* Start SILO with `SPDLOG_LEVEL=off` to turn off logging.
* Start SILO with `SPDLOG_LEVEL=debug` to log at debug level.

SILO will log to `./logs/silo_<date>.log` and to stdout.

We decided to use the macros provided by spdlog rather than the functions, because this lets us disable log statements
at compile time by adjusting `add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)` to the desired log level
via CMake. This might be desirable for benchmarking SILO. However, the default should be `SPDLOG_LEVEL_TRACE` to give
the maintainer the possibility to adjust the log level to a log level that they prefer, without the need to recompile
SILO.

# Code Style Guidelines

## Naming

We mainly follow the styleguide provided by [google](https://google.github.io/styleguide/cppguide.html), with a few
additions. The naming is enforced by clang-tidy. Please refer to `.clang-tidy` for more details on naming inside the
code. Clang-tidy can not detect filenames. We decided to use snake_case for filenames.

## Includes

The includes are sorted in the following order:

1. Corresponding header file (for source files)
2. System includes
3. External includes
4. Internal includes

Internal includes are marked by double quotes. External includes are marked by angle brackets.

## Conventional Commits

We follow the [conventional commits](https://www.conventionalcommits.org/) guidelines for commit messages.
This will allow to automatically generate a changelog.

We use [commitlint](https://commitlint.js.org/) to enforce the commit message format.
To use it locally, run `npm install`.

The last commit message can be checked with

```shell
npm run commitlint:last-commit
``` 

To check commit messages of a branch to the commit where it branches off from `main`, run

```shell
npm run commitlint:merge-base
```
