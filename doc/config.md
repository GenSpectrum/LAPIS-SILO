# How the configuration system works

SILO takes configuration information from 3 configuration sources:
YAML files, environment variables, and command line arguments. The
same variables can be defined via any of them (but while the path to
the first-level configuration file can even be defined in the file
itself, only values passed by env variable or command line are useful,
of course). Environment variables override YAML file entries, and
command line arguments override both.

The system works off metadata on the structs making up the
configuration data. 

The metadata is converted at runtime (via
[`ConfigStruct`](../include/config/config_specification.h)) to a flat
representation, a vector of tuples of
[`ConfigKeyPath`](../include/config/config_key_path.h) (list of key segment strings) and
reference to [`ConfigValue`](../include/config/config_specification.h) (the metadata on a
struct field).  This vector is the basis to build the help text,
or to map to vectors or key/value representations for the source
in question.

Each source ([command line arguments](XX), [environment variables](XX),
[yaml file](XX)) has its individual constructor and error handling
during construction. The resulting object must implement
[`VerifyConfigSource`](../include/config/config_specification.h), the `verify` method of
which takes the config values vector mentioned in the previous
paragraph, and returns an object that implements
[`VerifiedConfigSource`](../include/config/config_backend.h). This is then, inside
[`raw_get_config`](XX), passed to the
[`OverwriteFrom::overwrite_from`](XX?) method to
fill the fields of the to-be configured struct with the values
destined for them.

To make this work, each configurable struct needs to implement
[`OverwriteFrom`](XX?), additionally, the top-level
configurable struct needs to implement
[`ToplevelConfig`](XX). To provide that latter
implementation, the top-level config struct should have a boolean
help field, and a field to take a path to the config file that
should be read, if given.

The process of going through the 3 sources, and reading the config
file that was specified by the user, is handled by the
aforementiond `raw_get_config` function. All this
function needs is a reference to the (remaining) command line
arguments to be parsed, and a reference to the struct metadata for
the toplevel configuration struct. It returns the filled-in
struct, of the given type parameter which must match the metadata
that was given.

For more information (with quite some overlap with this description),
see [`config_source_interface`](../include/config/config_backend.h).
