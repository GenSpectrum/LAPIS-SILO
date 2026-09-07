#include <fmt/format.h>
#include <gtest/gtest.h>
#include <cctype>
#include <cstdlib>

#include "rhydb/common/panic.h"

namespace {
// Fuzzy comparison: `expected` should be without the file:line
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

void nullCapableSetenv(const char* name, const char* value, int overwrite) {
   if (value) {
      setenv(name, value, overwrite);
   } else {
      unsetenv(name);
   }
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, assertEqPanicModes) {
   RHYDB_ASSERT_EQ(1 + 1, 2);

   const char* old_env = getenv("RHYDB_PANIC");
   setenv("RHYDB_PANIC", "", 1);
   try {
      RHYDB_ASSERT_EQ(1 + 1, 3);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "ASSERT_EQ failure: 1 + 1 == 3: 2 == 3");
   };

   setenv("RHYDB_PANIC", "abort", 1);
   ASSERT_DEATH(RHYDB_ASSERT_EQ(1 + 1, 3), "ASSERT_EQ failure: 1 \\+ 1 == 3: 2 == 3");

   // revert it back
   nullCapableSetenv("RHYDB_PANIC", old_env, 1);
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, debugAssertBehavesAsPerCompilationMode) {
   // should never complain
   RHYDB_DEBUG_ASSERT(1 + 1 == 2);

   // Check that RHYDB_DEBUG_ASSERT is active if RHYDB_DEBUG_ASSERTIONS==1, off
   // otherwise; each of those branches is only tested when compiling
   // the unit tests in debug or release mode, respectively.

#if RHYDB_DEBUG_ASSERTIONS

   const char* old_env = getenv("RHYDB_PANIC");
   setenv("RHYDB_PANIC", "", 1);
   try {
      RHYDB_DEBUG_ASSERT(1 + 1 == 3);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "DEBUG_ASSERT failure: 1 + 1 == 3");
   };
   nullCapableSetenv("RHYDB_PANIC", old_env, 1);

#else
   // check that RHYDB_DEBUG_ASSERT is disabled
   RHYDB_DEBUG_ASSERT(1 + 1 == 3);
#endif
}

// NOLINTNEXTLINE(readability-identifier-naming,readability-function-cognitive-complexity)
TEST(panic, debugAssertGeWorks) {
   // stand-in for all the RHYDB_DEBUG_ASSERT_* variants

   RHYDB_DEBUG_ASSERT_GE(1 + 5, 6);
   RHYDB_DEBUG_ASSERT_GE(1 + 5, 5);

#if RHYDB_DEBUG_ASSERTIONS

   const char* old_env = getenv("RHYDB_PANIC");
   setenv("RHYDB_PANIC", "", 1);
   try {
      RHYDB_DEBUG_ASSERT_GE(1 + 5, 7);
   } catch (const std::exception& ex) {
      assertMsg(ex.what(), "DEBUG_ASSERT_GE failure: 1 + 5 >= 7: 6 >= 7");
   };
   nullCapableSetenv("RHYDB_PANIC", old_env, 1);

#else
   // check that RHYDB_DEBUG_ASSERT is disabled
   RHYDB_DEBUG_ASSERT_GE(1 + 5, 7);
#endif
}
