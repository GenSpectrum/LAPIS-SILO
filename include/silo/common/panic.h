#pragma once

#include <fmt/format.h>

namespace silo::common {

/// ** This is the basic building block for the various panicking
/// features offered in this file. You probably want to use the
/// `PANIC` macro instead of `panic` directly, to get format
/// functionality and file/line number information! **
///
/// `panic` stops execution because of a situation that the author of
/// the program expected to never happen, i.e. that is against
/// expectations of internal consistency, and means that there is
/// either a bug, or a misunderstanding of the consistency rules on
/// behalf of the programmer.
///
/// By default, `panic` throws a `std::runtime_error` exception, so
/// that the program can continue by e.g. abandoning the current http
/// connection but continue servicing other and future
/// connections. OTOH, to make debugging via gdb or core dumps
/// possible even when the code that captures exceptions can't be
/// disabled, `panic` can be instructed at runtime to call `abort`
/// instead by setting the `SILO_PANIC` environment variable to the
/// string `abort` (with any other value, or when unset, panic
/// silently throws the mentioned exception instead).
[[noreturn]] void panic(const std::string& msg, const char* file, int line);

/// Passes arguments to `fmt::format` (at least a format string
/// argument is required) and adds file and line information, then
/// calls `panic`.
#define PANIC(...) silo::common::panic(fmt::format(__VA_ARGS__), __FILE__, __LINE__)

/// Denotes a place that theoretically can't be reached. Follows the
/// same path as `PANIC` when reached.
#define UNREACHABLE() silo::common::unreachable(__FILE__, __LINE__)

[[noreturn]] void unreachable(const char* file, int line);

/// Denotes a missing implementation. Follows the same path as `PANIC`
/// when reached.
#define UNIMPLEMENTED() silo::common::unimplemented(__FILE__, __LINE__)

[[noreturn]] void unimplemented(const char* file, int line);

/// Asserts that the expression `e` evaluates to true. On failure
/// calls `panic` with the stringification of the code `e` and
/// file/line information.
#define ASSERT(e)                                             \
   do {                                                       \
      if (!(e)) {                                             \
         silo::common::assertFailure(#e, __FILE__, __LINE__); \
      }                                                       \
   } while (0)

[[noreturn]] void assertFailure(const char* msg, const char* file, int line);

}  // namespace silo::common
