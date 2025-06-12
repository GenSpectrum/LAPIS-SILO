# How the configuration system works

SILO takes configuration information from three sources:
YAML files, environment variables, and command line arguments. The
same variables can be defined via any of them (except the path to the
config file only makes sense to be defined via environment variable or
command line option, of course). Environment variables override YAML
file entries, and command line arguments override both.

For a class representing application configuration data (from here on
called "Config", because the config library defines a C++ concept
called [`Config`](../src/config/config_interface.h) that the
"Config" class needs to implement--currently those are
`PreprocessingConfig` or `RuntimeConfig`), the system needs metadata,
represented with the
[`ConfigSpecification`](../src/config/config_specification.h)
type. This metadata is the basis for building the help text, and used
by the configuration source "backends" (via the `verify` method
implementations of the [`ConfigSource`
interface](../src/config/config_source_interface.h)) to know which
user-provided values are valid, and what type they represent (which is
used in the help text and to choose the right parser).

As per the [`Config`](../src/config/config_interface.h) concept, a
"Config" needs to implement an `overwriteFrom` method, which receives
the parsed and verified user-provided data from one of the config
sources, and has to fill in all class fields with the values given by
the source. `overwriteFrom` is called for each config source on the
same "Config" instance, each next one overwriting (shadowing) the
value from the former source.

Each configuration source has an initial representation, before the
`verify` step happens, which represents the information provided by
the user without configuration-specific parsing. Its creation takes
source-specific arguments and can have source-specific errors: the
YAML source parses the given configuration file with a YAML parser,
throwing exceptions for YAML parsing errors.

There are some complications:

* Command line arguments can only be parsed given the
  `ConfigSpecification`, because boolean command line options do not
  take a value after them, thus how the next argument is to be
  processed depends on that information. This is why only the `verify`
  step can find out which positional arguments exist, and why
  `VerifiedConfigAttributes` also carries positional arguments (which are
  the empty vector for backends that do not have them).

* The configuration file path must be read from environment variables or
  command line arguments first. Then the file can be parsed, but then
  its values need to be shadowed again by values given via environment
  variables or command line arguments.

* Command line arguments should be read first, to get the `--help`
  option, to avoid erroring out and stopping while reading environment
  variables.

* SILO needs to be able to receive two config files: one with default
  values for an environment (for a Docker image), and another one
  optionally provided by the user of a Docker image; values in the latter
  should shadow values in the former. For this reason,
  [`Config`](../src/config/config_interface.h) requires a
  `getConfigPaths` method that returns the paths to the default, and
  if given, user-provided config file (taken from (defaults,) env vars
  and command line options).

* Allowing multiple modes (with `silo` currently `api` and
  `preprocessing`), while also allowing configuration via environment
  variables, requires that environment variables meant for the other
  mode should not lead to errors. Environment variables are often set
  before deciding in which mode the program should run, or hang
  around, or in the case of the Docker image the configuration files
  are always specified via env vars, for both modes. However, while
  totally unknown environment variables that start with the
  application prefix (`SILO_`) should be reported as usage errors.

The logic for handling the precedence and these complications is in
the templated [`getConfig`](../src/config/config_interface.h)
function, which returns a fully filled-in "Config"
instance. `getConfig` needs an allow list for environment variables to
satisfy the last point above; this is done in
[`main.cpp`](../src/main.cpp).

Hard-coded values for a "Config" class should be read from its
`ConfigSpecification`, in its constructor, rather than using the `=`
syntax in the class definition, to maintain a single source of truth for both
the help text and the actual runtime.

## Code Overview

[src/config/config_interface.h](../src/config/config_interface.h)

The `Config` concept, and the `getConfig<Config>` function.

[src/config/config_specification.h](../src/config/config_specification.h)

`ConfigSpecification`, describing the "metadata" on a Config class
using a `ConfigAttributeSpecification` for each field.

[src/config/verified_config_attributes.h](../src/config/verified_config_attributes.h)

`VerifiedConfigAttributes`: the result of the verification (`verify` method), input for `getConfig`.

[src/config/config_key_path.h](../src/config/config_key_path.h)

`ConfigKeyPath`: source-independent abstraction for a configuration key.

[src/config/config_value.h](../src/config/config_value.h)

`ConfigValue`: A parsed value from user config input.

[src/config/config_source_interface.h](../src/config/config_source_interface.h)

Declares the `verify` method.

[src/config/source/](../src/config/source/)

Contains the source "backends".

<img src="config.svg">

