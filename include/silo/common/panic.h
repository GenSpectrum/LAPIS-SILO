#pragma once

#include <fmt/format.h>

namespace silo::common {

void panic(const std::string& msg, const char* file, int line);

/// Passes arguments to `fmt::format` (at least a format string
/// argument is required), then if the `DEBUG` environment variable is
/// set to anything else than the string "0", the resulting string is
/// printed to stderr and `abort` is called, otherwise a
/// `std::runtime_error` is thrown (details subject to
/// change). `PANIC` should only be used in situations that can never
/// occur unless there is a bug; the aim, besides allowing the
/// programmer to declare clearly what constitutes buggy behaviour, is
/// to allow these situations to be debugged via a debugger like gdb
/// without needing contortions like setting breakpoints, or also to
/// allow the collection of core dumps of these situations.
#define PANIC(...) silo::common::panic(fmt::format(__VA_ARGS__), __FILE__, __LINE__)

}  // namespace silo::common
