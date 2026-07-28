#pragma once

#include <optional>

#include <Poco/Timer.h>

namespace silo_app {

class MemoryMonitor {
   std::optional<uint32_t> soft_memory_limit_in_kb;
   Poco::Timer timer;

  public:
   explicit MemoryMonitor(std::optional<uint32_t> soft_memory_limit_in_kb);

   void checkRssAndLimit(Poco::Timer& /*timer*/);
};

}  // namespace silo_app
