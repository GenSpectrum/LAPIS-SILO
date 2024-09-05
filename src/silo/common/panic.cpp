#include "silo/common/panic.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace silo::common {

namespace {

[[noreturn]] void panic(
   const std::string& prefix,
   const std::string& msg,
   const char* file,
   int line
) {
   const char* env = getenv("SILO_PANIC");
   bool do_abort;
   if (env) {
      do_abort = strcmp(env, "abort") == 0;
   } else {
      do_abort = false;
   }
   auto full_msg = fmt::format("{}{} at {}:{}", prefix, msg, file, line);
   if (do_abort) {
      std::cerr << full_msg << "\n" << std::flush;
      abort();
   } else {
      throw std::runtime_error(full_msg);
   }
}

}  // namespace

[[noreturn]] void panic(const std::string& msg, const char* file, int line) {
   panic("PANIC: ", msg, file, line);
}

[[noreturn]] void unreachable(const char* file, int line) {
   panic(
      "UNREACHABLE: ",
      "Please report this as a bug in SILO: this code should never be reachable",
      file,
      line
   );
}

[[noreturn]] void unimplemented(const char* file, int line) {
   panic("UNIMPLEMENTED: ", "This execution path is not implemented yet", file, line);
}

[[noreturn]] void assertFailure(const char* msg, const char* file, int line) {
   panic("ASSERT failure: ", msg, file, line);
}

[[noreturn]] void debugAssertFailure(const char* msg, const char* file, int line) {
   panic("DEBUG_ASSERT failure: ", msg, file, line);
}

}  // namespace silo::common
