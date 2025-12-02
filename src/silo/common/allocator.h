#ifdef SILO_USE_MIMALLOC
#include <mimalloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

#include <spdlog/spdlog.h>

namespace silo::common {

class Allocator {
  public:
#ifdef SILO_USE_MIMALLOC
   static void trim() {
      SPDLOG_INFO("Manually invoking mi_collect(true) to give back memory to OS.");
      mi_collect(true);
      mi_collect(true);  // This should be invoked twice
   }
#elif defined(__linux__)
   static void trim() {
      SPDLOG_INFO("Manually invoking malloc_trim() to give back memory to OS.");
      malloc_trim(0);
   }
#else
   static void trim() { SPDLOG_INFO("Allocator::trim() is not implemented for this platform."); }
#endif
};

}  // namespace silo::common