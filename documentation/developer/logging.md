## Logging

We use [spdlog](https://github.com/gabime/spdlog) for logging.
The log level can be controlled via the environment variable `SPDLOG_LEVEL`:

- Start RhyDB with `SPDLOG_LEVEL=off` to turn off logging.
- Start RhyDB with `SPDLOG_LEVEL=debug` to log at debug level.

RhyDB logs to a daily rotating file at `./logs/silo.log` and to stdout.

We decided to use the macros provided by spdlog rather than the functions, because this lets us disable log statements
at compile time by adjusting `add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)` to the desired log level
via CMake. This might be desirable for benchmarking RhyDB. However, the default should be `SPDLOG_LEVEL_TRACE` to give
the maintainer the possibility to adjust the log level to a log level that they prefer, without the need to recompile
RhyDB.
