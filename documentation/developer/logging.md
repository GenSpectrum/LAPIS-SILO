## Logging

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
