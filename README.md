# LAPIS-SILO

Sequence Indexing engine for Large Order of genomic data

# License

Original genome indexing logic with roaring bitmaps by Prof. Neumann: https://db.in.tum.de/~neumann/gi/

# Building

Use `./build_with_conan.py` to build SILO. `./build_with_conan.py --help` shows all available options.

The conan center has been updated, if you installed conan before November 2024 you might need to update your center: `conan remote update conancenter --url="https://center2.conan.io"`

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

### Formatting

We use clang-format as a code formatter. To run locally install [clang-format](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6), update your PATH variables and run

```bash
find src -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
```

Note that your clang-format version should be exactly the same as that used by `jidicula/clang-format-action` for tests to pass. Currently we use `v.17.0.6`.

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
  file [database_config.h](src/silo/config/database_config.h)
- `PreprocessingConfig` used when started with `preprocessing` and described in
  file [preprocessing_config.h](src/silo/config/preprocessing_config.h)
- `RuntimeConfig` used when started with `api` and described in
  file [runtime_config.h](src/silo/config/preprocessing_config.h)

The database config contains the schema of the database and is always required when preprocessing data. The database
config will be saved together with the output of the preprocessing and is therefore not required when starting SILO as
an API.
An example of a configuration file can be seen
in [testBaseData/exampleDataset/database_config.yaml](testBaseData/exampleDataset/database_config.yaml).

By default, the config files are expected to be YAML files in the current working directory in
snake_case (`database_config.yaml`, `preprocessing_config.yaml`, `runtime_config.yaml`), but their location can be
overridden using the options `--database-config=X`, `--preprocessing-config=X`, and `--runtime-config=X`.

Preprocessing and Runtime configurations contain default values for all fields and are thus only optional. Their
parameters can also be provided as command-line arguments in snake_case and as environment variables prefixed with SILO_
in capital SNAKE_CASE. (e.g. SILO_INPUT_DIRECTORY).

The precendence is `CLI argument > Environment Variable > Configuration File > Default Value`

### Run The Preprocessing

The preprocessing acts as a program that takes an input directory that contains the to-be-processed data
and an output directory where the processed data will be stored.
Both need to be mounted to the container.

SILO expects a preprocessing config that can to be mounted to the default location `/app/preprocessing_config.yaml`.

Additionally, a database config and a ndjson file containing the data are required. They should typically be mounted in `/preprocessing/input`.

```shell
docker run \
  -v your/input/directory:/preprocessing/input \
  -v your/preprocessing/output:/preprocessing/output \
  -v your/preprocessing_config.yaml:/app/preprocessing_config.yaml
  silo preprocessing
```

Both config files can also be provided in custom locations:

```shell
silo preprocessing --preprocessing-config=./custom/preprocessing_config.yaml --database-config=./custom/database_config.yaml
```

The Docker image contains a default preprocessing config that sets defaults specific for running SILO in Docker.
Apart from that, there are default values if neither user-provided nor default config specify fields.
The user-provided preprocessing config can be used to overwrite the default values. For a full reference,
see the help text.

### Run docker container (api)

After preprocessing the data, the api can be started with the following command:

```shell
docker run
  -p 8081:8081
  -v your/preprocessing/output:/data
  silo api
```

The directory where SILO expects the preprocessing output can be overwritten via
`silo api --data-directory=/custom/data/directory` or in a corresponding
[configuration file](#configuration-files).

### Notes On Building The Image

Building Docker images locally relies on the local Docker cache.
Docker will cache layers, and it will cache the dependencies built by Conan via cache mounts.

However, cache mounts don't work in GitHub Actions (https://github.com/docker/build-push-action/issues/716),
so there we only rely on Docker's layer cache via Docker's gha cache backend.

## Creating A Release

This project uses [Release Please](https://github.com/google-github-actions/release-please-action) to generate releases.
On every commit on the `main` branch, it will update a Pull Request with a changelog.
When the PR is merged, the release will be created.
Creating a release means:

- A new Git tag is created.
- The Docker images of SILO are tagged with the new version.
  - Suppose the created version is `2.4.5`, then it creates the tags `2`, `2.4` and `2.4.5` on the current `latest` image.

The changelog and the version number are determined by the commit messages.
Therefore, commit messages should follow the [Conventional Commits](https://www.conventionalcommits.org/) specification.
Also refer to the Release Please documentation for more information on how to write commit messages
or see [Conventional Commits](#conventional-commits) below.

# Testing

Before committing, run `make ci` to execute the formatter and all tests (unit and e2e) locally.

## Unit Tests

For testing, we use the framework [gtest](http://google.github.io/googletest/)
and [gmock](http://google.github.io/googletest/gmock_cook_book.html) for mocking. Tests are built using the same script
as the production code: `./build_with_conan`.

We use the convention, that each tested source file has its own test file, ending with `*.test.cpp`. The test file is
placed in the same folder as the source file. If the function under test is described in a header file, the test file is
located in the corresponding source folder.

To run all tests, run

```shell
build/Release/silo_test
```

For linting we use clang-tidy. The config is stored in `.clang-tidy`.

When pushing to GitHub, a separate Docker image will be built, which runs the formatter. (This is a workaround, because
building with clang-tidy under alpine was not possible yet.)

## Functional End-To-End Tests

End-to-end tests are located in `/endToEndTests`. Those tests are used to verify the overall functionality of the SILO
queries. To execute the tests:

- have a running SILO instance with preprocessd data e.g. via
  - `SILO_IMAGE=ghcr.io/genspectrum/lapis-silo docker compose -f docker-compose-for-tests-preprocessing.yml up`
  - `SILO_IMAGE=ghcr.io/genspectrum/lapis-silo docker compose -f docker-compose-for-tests-api.yml up -d wait`
- `cd endToEndTests`
- `npm install`
- `SILO_URL=localhost:8081 npm run test`

# Local Debugging

We recommend using [LLDB](https://lldb.llvm.org/) with Cmake for local debugging.

If you are using VSCode we recommend installing the extensions listed in the `.vscode.extensions.json`. This will add a new icon for Cmake, to debug using Cmake and LLDB first configure the project (by selecting configure in the Cmake panel) and update the Cmake `settings.json` to use LLDB. This means adding the following to your settings.json.
```
"cmake.debugConfig": {
    "MIMode": "lldb"
  }
```

# Logging

We use [spdlog](https://github.com/gabime/spdlog) for logging.
The log level can be controlled via the environment variable `SPDLOG_LEVEL`:

- Start SILO with `SPDLOG_LEVEL=off` to turn off logging.
- Start SILO with `SPDLOG_LEVEL=debug` to log at debug level.

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

Please make sure to mention a reference in the commit message so that the generated changelog can be linked to
either an issue or a pull request.
This can be done via:

- Referencing an issue via "resolves" to the commit footer (preferred solution):

```
feat: my fancy new feature

some description

resolves #123
```

- Referencing an issue in the commit message header: `feat: my fancy new feature (#123)`
- Squash-merging on GitHub and adding the PR number to the commit message
  (useful for smaller changes that don't have a corresponding issue).

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

### Testing The Generated Changelog

To test the generated changelog, run

```shell
npm run release-please-dry-run -- --token=<GitHub PAT> --target-branch=<name of the upstream branch>
```

where

- `<GitHub PAT>` is a GitHub Personal Access Token. It doesn't need any permissions.
- `<name of the upstream branch>` is the name of the branch for which the changelog should be generated.

**NOTE: This command does not respect local changes. It will pull the commit messages from the remote repository.**
