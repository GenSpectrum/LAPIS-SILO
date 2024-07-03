#include "silo/common/panic.h"

#include <cstdlib>
#include <iostream>

namespace silo::common {

namespace {
bool is0(const char* str) {
   return str[0] == '0' && str[1] == '\0';
}
}  // namespace

void panic(const std::string& msg, const char* file, int line) {
   const char* env = getenv("DEBUG");
   bool debug;
   if (env) {
      debug = !is0(env);
   } else {
      debug = false;
   }
   auto full_msg = fmt::format("PANIC: {} at {}:{}", msg, file, line);
   if (debug) {
      std::cerr << full_msg << "\n" << std::flush;
      abort();
   } else {
      throw std::runtime_error(full_msg);
   }
}

}  // namespace silo::common
