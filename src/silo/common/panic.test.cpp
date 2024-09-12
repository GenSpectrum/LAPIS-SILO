#include <fmt/format.h>
#include <gtest/gtest.h>
#include <cctype>
#include <cstdlib>

/* get rid of gtest's definition, we're going to test our own */
#undef ASSERT_EQ

#include "silo/common/panic.h"

namespace {
// Since we can't use ASSERT_EQ from gtest, make a simple
// replacement. `expected` should be without the file:line
// information, whereas `got` should contain it.
void assertMsg(std::string got, std::string expected) {
   auto start = got.substr(0, expected.size());
   if (start != expected) {
      throw std::runtime_error(
         fmt::format("expected '{}', got '{}' (full: '{}')", expected, start, got)
      );
   }
   auto remainder = got.substr(expected.size(), got.size() - expected.size());
   if (remainder.size() < 4) {
      throw std::runtime_error("missing ' at ..' part of exception message");
   }
   auto the_at = remainder.substr(0, 4);
   if (the_at != std::string(" at ")) {
      throw std::runtime_error(fmt::format("expected '{}', got '{}'", " at ", the_at));
   }

   const char last = remainder[remainder.size() - 1];
   if (!isdigit(last)) {
      throw std::runtime_error(fmt::format("expected a digit at the end of '{}'", got));
   }
   // good enough, ignore the rest of the remainder.
}
}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, assertEqPanicModes) {
   ASSERT_EQ(1 + 1, 2);

   setenv("SILO_PANIC", "", 1);
   try {
      ASSERT_EQ(1 + 1, 3);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "ASSERT_EQ failure: 1 + 1 == 3: 2 == 3");
   };

   setenv("SILO_PANIC", "abort", 1);
   ASSERT_DEATH(ASSERT_EQ(1 + 1, 3), "ASSERT_EQ failure: 1 \\+ 1 == 3: 2 == 3");

   // revert it back
   setenv("SILO_PANIC", "", 1);
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, debugAssertBehavesAsPerCompilationMode) {
   // should never complain
   DEBUG_ASSERT(1 + 1 == 2);

   // Check that DEBUG_ASSERT is active if DEBUG_ASSERTIONS==1, off
   // otherwise; each of those branches is only tested when compiling
   // the unit tests in debug or release mode, respectively.

#if DEBUG_ASSERTIONS

   setenv("SILO_PANIC", "", 1);
   try {
      DEBUG_ASSERT(1 + 1 == 3);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "DEBUG_ASSERT failure: 1 + 1 == 3");
   };

#else
   // check that DEBUG_ASSERT is disabled
   DEBUG_ASSERT(1 + 1 == 3);
#endif
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, debugAssertGeWorks) {
   // stand-in for all the DEBUG_ASSERT_* variants

   DEBUG_ASSERT_GE(1 + 5, 6);
   DEBUG_ASSERT_GE(1 + 5, 5);

#if DEBUG_ASSERTIONS

   setenv("SILO_PANIC", "", 1);
   try {
      DEBUG_ASSERT_GE(1 + 5, 7);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "DEBUG_ASSERT_GE failure: 1 + 5 >= 7: 6 >= 7");
   };

#else
   // check that DEBUG_ASSERT is disabled
   DEBUG_ASSERT_GE(1 + 5, 7);
#endif
}
