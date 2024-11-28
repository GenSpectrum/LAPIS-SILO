# How the configuration system works

SILO takes configuration information from 3 configuration sources:
YAML files, environment variables, and command line arguments. The
same variables can be defined via any of them (except the path to the
config file only makes sense to be defined via environment variable or
command line option, of course). Environment variables override YAML
file entries, and command line arguments override both.

For a struct representing application configuration data (from here on called "Config", because there is a C++ concept called [`Config`](XXX) that it needs to implement--currently those are `PreprocessingConfig` or `RuntimeConfig`), the system needs metadata, represented with
the [`ConfigSpecification`](XXX) type. This metadata is the basis for
building the help text, and used by the configuration source
"backends" (in the `verify` method implementations of the
[`ConfigSource` interface](include/config/config_source_interface.h))
to know which user-provided values are valid, and what type they
represent (which is used to choose the right parser).

As per the `Config` concept, a "Config" needs to implement an
`overwriteFrom` method, which receives the parsed and verified
user-provided data from one of the config sources, and has to fill in
all struct fields with the values given by the source.

Each configuration source has a first step, before the `verify` step
happens, that retrieves the information provided by the user without
configuration-specific parsing (e.g. parse the configuration file with
a Yaml parser). That first step takes source-specific arguments and
can have source-specific errors: Yaml parsing throws exceptions for
Yaml parsing errors.


There are some complications:

* boolean  options +   command line   -> only in verify posisible  XXX

* configuration file path must be read from environment variables or
  command line arguments first, then the file can be parsed, but then
  its values need to be shadowed again by values given via environment
  variables or command line arguments.

* command line arguments should be read first, to get the `--help`
  option, to avoid erroring out and stopping while reading environment
  variables.

The logic for handling the precedence and these complications is in
the templated [`getConfig`](XXX) function, which returns a fully
filled-in "Config" instance.

XXX
For more information (with quite some overlap with this description),
see [`config_source_interface`](../include/config/config_source_interface.h).
