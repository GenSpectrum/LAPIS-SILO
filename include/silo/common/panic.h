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
/// file/line information. `ASSERT` is always compiled in; if
/// performance overrides safety, use `DEBUG_ASSERT` instead.
#define ASSERT(e)                                             \
   do {                                                       \
      if (!(e)) {                                             \
         silo::common::assertFailure(#e, __FILE__, __LINE__); \
      }                                                       \
   } while (0)

[[noreturn]] void assertFailure(const char* msg, const char* file, int line);

/// `DEBUG_ASSERT` is like `ASSERT`, but for cases where performance
/// is more important than verification in production: instantiations
/// are only active when compiling SILO in debug (via
/// `CMakeLists.txt`; concretely, they are compiled to be active when
/// the preprocessor variable `DEBUG_ASSERTIONS` is set to 1, and
/// ignored if that variable is set to 0; if the variable is missing,
/// a compilation warning is printed and `DEBUG_ASSERT` is ignored, if
/// present with another value, a compilation error results. Note that
/// `DEBUG_ASSERTIONS` must be set to 1 for debug builds or
/// `DEBUG_ASSERT` won't even check the assertion in debug builds. The
/// SILO `CMakeLists.txt` does set it up that way.)
#ifndef DEBUG_ASSERTIONS
#warning \
   "DEBUG_ASSERTIONS is not set, should be 0 to ignore DEBUG_ASSERT, 1 to compile it in, assuming 0"
#define DEBUG_ASSERTIONS 0
#else
#if DEBUG_ASSERTIONS == 0   /* never */
#elif DEBUG_ASSERTIONS == 1 /* always */
#else
#error "DEBUG_ASSERTIONS should be 0 to ignore DEBUG_ASSERT, 1 to compile it in"
#endif
#endif

#define DEBUG_ASSERT(e)                                               \
   do {                                                               \
      if (DEBUG_ASSERTIONS) {                                         \
         if (!(e)) {                                                  \
            silo::common::debugAssertFailure(#e, __FILE__, __LINE__); \
         }                                                            \
      }                                                               \
   } while (0)

[[noreturn]] void debugAssertFailure(const char* msg, const char* file, int line);

}  // namespace silo::common
