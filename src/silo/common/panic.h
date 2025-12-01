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
#define SILO_PANIC(...) silo::common::panic(fmt::format(__VA_ARGS__), __FILE__, __LINE__)

/// Denotes a place that isn't implemented *yet*, during
/// development. Follows the same path as `PANIC` when reached.
#define SILO_TODO() silo::common::todo(__FILE__, __LINE__)

[[noreturn]] void todo(const char* file, int line);

/// Denotes a place that theoretically can't be reached. Follows the
/// same path as `PANIC` when reached.
#define SILO_UNREACHABLE() silo::common::unreachable(__FILE__, __LINE__)

[[noreturn]] void unreachable(const char* file, int line);

/// Denotes a missing implementation. Follows the same path as `PANIC`
/// when reached.
#define SILO_UNIMPLEMENTED() silo::common::unimplemented(__FILE__, __LINE__)

[[noreturn]] void unimplemented(const char* file, int line);

/// Asserts that the expression `e` evaluates to true. On failure
/// calls `panic` with the stringification of the code `e` and
/// file/line information. `SILO_ASSERT` is always compiled in; if
/// performance overrides safety, use `SILO_DEBUG_ASSERT` instead.
#define SILO_ASSERT(e)                                        \
   do {                                                       \
      if (!(e)) {                                             \
         silo::common::assertFailure(#e, __FILE__, __LINE__); \
      }                                                       \
   } while (0)

[[noreturn]] void assertFailure(const char* msg, const char* file, int line);

#define SILO_INTERNAL_ASSERT_OP_(prefix_str, e1, op, e2)                                 \
   do {                                                                                  \
      auto silo_internal_assert_op__v1 = (e1);                                           \
      auto silo_internal_assert_op__v2 = (e2);                                           \
      if (!(silo_internal_assert_op__v1 op silo_internal_assert_op__v2)) {               \
         silo::common::assertOpFailure(                                                  \
            prefix_str,                                                                  \
            #e1,                                                                         \
            #op,                                                                         \
            #e2,                                                                         \
            fmt::format(                                                                 \
               "{} " #op " {}", silo_internal_assert_op__v1, silo_internal_assert_op__v2 \
            ),                                                                           \
            __FILE__,                                                                    \
            __LINE__                                                                     \
         );                                                                              \
      }                                                                                  \
   } while (0)

#define SILO_ASSERT_OP_(partial_prefix, e1, op, e2) \
   SILO_INTERNAL_ASSERT_OP_("ASSERT_" #partial_prefix, e1, op, e2)

[[noreturn]] void assertOpFailure(
   const char* prefix,
   const char* e1_str,
   const char* op_str,
   const char* e2_str,
   const std::string& values,
   const char* file,
   int line
);

/// Asserts that the two given expressions evaluate to the same
/// value, compared via `==`. On failure calls `panic` with the stringification of the code and the
/// two values passed through fmt::format, plus file/line information. Always compiled in, if
/// performance overrides safety, use `DEBUG_ASSERT_EQ` instead.
#define SILO_ASSERT_EQ(e1, e2) SILO_ASSERT_OP_(EQ, e1, ==, e2)

/// Like ASSERT_EQ but asserts that `e1 <= e2`.
#define SILO_ASSERT_LE(e1, e2) SILO_ASSERT_OP_(LE, e1, <=, e2)

/// Like ASSERT_EQ but asserts that `e1 < e2`.
#define SILO_ASSERT_LT(e1, e2) SILO_ASSERT_OP_(LT, e1, <, e2)

/// Like ASSERT_EQ but asserts that `e1 >= e2`.
#define SILO_ASSERT_GE(e1, e2) SILO_ASSERT_OP_(GE, e1, >=, e2)

/// Like ASSERT_EQ but asserts that `e1 > e2`.
#define SILO_ASSERT_GT(e1, e2) SILO_ASSERT_OP_(GT, e1, >, e2)

/// Like SILO_ASSERT_EQ but asserts that `e1 != e2`.
#define SILO_ASSERT_NE(e1, e2) SILO_ASSERT_OP_(NE, e1, !=, e2)

/// `SILO_DEBUG_ASSERT` is like `SILO_ASSERT`, but for cases where performance
/// is more important than verification in production: instantiations
/// are only active when compiling SILO in debug (via
/// `CMakeLists.txt`; concretely, they are compiled to be active when
/// the preprocessor variable `SILO_DEBUG_ASSERTIONS` is set to 1, and
/// ignored if that variable is set to 0; if the variable is missing,
/// a compilation warning is printed and `SILO_DEBUG_ASSERT` is ignored, if
/// present with another value, a compilation error results. Note that
/// `SILO_DEBUG_ASSERTIONS` must be set to 1 for debug builds or
/// `SILO_DEBUG_ASSERT` won't even check the assertion in debug builds. The
/// SILO `CMakeLists.txt` does set it up that way.)
#ifndef SILO_DEBUG_ASSERTIONS
#warning \
   "SILO_DEBUG_ASSERTIONS is not set, should be 0 to ignore SILO_DEBUG_ASSERT, 1 to compile it in, assuming 0"
#define SILO_DEBUG_ASSERTIONS 0
#else
#if SILO_DEBUG_ASSERTIONS == 0   /* never */
#elif SILO_DEBUG_ASSERTIONS == 1 /* always */
#else
#error "SILO_DEBUG_ASSERTIONS should be 0 to ignore SILO_DEBUG_ASSERT, 1 to compile it in"
#endif
#endif

#define SILO_DEBUG_ASSERT(e)                                          \
   do {                                                               \
      if (SILO_DEBUG_ASSERTIONS) {                                    \
         if (!(e)) {                                                  \
            silo::common::debugAssertFailure(#e, __FILE__, __LINE__); \
         }                                                            \
      }                                                               \
   } while (0)

[[noreturn]] void debugAssertFailure(const char* msg, const char* file, int line);

#define SILO_DEBUG_ASSERT_OP_(partial_prefix, e1, op, e2)                       \
   do {                                                                         \
      if (SILO_DEBUG_ASSERTIONS) {                                              \
         SILO_INTERNAL_ASSERT_OP_("DEBUG_ASSERT_" #partial_prefix, e1, op, e2); \
      }                                                                         \
   } while (0)

/// Like `SILO_ASSERT_EQ`, but like `SILO_DEBUG_ASSERT`, for cases where
/// performance is more important than verification in production.
#define SILO_DEBUG_ASSERT_EQ(e1, e2) SILO_DEBUG_ASSERT_OP_(EQ, e1, ==, e2)

/// Like DEBUG_ASSERT_EQ but asserts that `e1 <= e2`.
#define SILO_DEBUG_ASSERT_LE(e1, e2) SILO_DEBUG_ASSERT_OP_(LE, e1, <=, e2)

/// Like DEBUG_ASSERT_EQ but asserts that `e1 < e2`.
#define SILO_DEBUG_ASSERT_LT(e1, e2) SILO_DEBUG_ASSERT_OP_(LT, e1, <, e2)

/// Like DEBUG_ASSERT_EQ but asserts that `e1 >= e2`.
#define SILO_DEBUG_ASSERT_GE(e1, e2) SILO_DEBUG_ASSERT_OP_(GE, e1, >=, e2)

/// Like DEBUG_ASSERT_EQ but asserts that `e1 > e2`.
#define SILO_DEBUG_ASSERT_GT(e1, e2) SILO_DEBUG_ASSERT_OP_(GT, e1, >, e2)

/// Like DEBUG_ASSERT_EQ but asserts that `e1 != e2`.
#define SILO_DEBUG_ASSERT_NE(e1, e2) SILO_DEBUG_ASSERT_OP_(NE, e1, !=, e2)

}  // namespace silo::common
