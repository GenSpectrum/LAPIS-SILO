#include "silo/api/memory_monitor.h"

#if defined(__linux__)

#include <malloc.h>

#include <fstream>
#include <optional>
#include <regex>
#include <string>

#include <spdlog/spdlog.h>

namespace {

std::optional<uint32_t> parseVmRSSLine(const std::string& line) {
   static const std::regex vmRssRegex("VmRSS:\\s*(\\d+) kB");
   std::smatch match;

   if (std::regex_search(line, match, vmRssRegex) && match.size() > 1) {
      try {
         return std::stol(match.str(1));
      } catch (const std::out_of_range& oor) {
         SPDLOG_DEBUG(
            "parseVmRSSLine: VmRSS value out of range for long: {} - {}", match.str(1), oor.what()
         );
         return std::nullopt;
      } catch (const std::invalid_argument& ia) {
         SPDLOG_DEBUG(
            "parseVmRSSLine: Invalid argument for stol: {} - {}", match.str(1), ia.what()
         );
         return std::nullopt;
      }
   }
   return std::nullopt;
}

std::optional<uint32_t> getResidentSetSize() noexcept {
   std::filesystem::path path = "/proc/self/status";
   std::ifstream file(path);
   if (!file.is_open()) {
      SPDLOG_DEBUG("getResidentSetSize: Could not open status file {}.", path.string());
      return std::nullopt;
   }

   std::string line;
   while (std::getline(file, line)) {
      if (line.rfind("VmRSS:", 0) == 0) {
         return parseVmRSSLine(line);
      }
   }

   SPDLOG_DEBUG("getResidentSetSize: VmRSS line not found in {}", path.string());
   return std::nullopt;
}

const long FIVE_SECONDS = 5000;

}  // namespace

namespace silo::api {

MemoryMonitor::MemoryMonitor(std::optional<uint32_t> soft_memory_limit_in_kb)
    : soft_memory_limit_in_kb(soft_memory_limit_in_kb),
      timer(0, FIVE_SECONDS) {
   timer.start(Poco::TimerCallback<MemoryMonitor>(*this, &MemoryMonitor::checkRssAndLimit));
}

void MemoryMonitor::checkRssAndLimit(Poco::Timer& /*timer*/) {
   auto rss = getResidentSetSize();
   if (rss.has_value()) {
      SPDLOG_INFO("Current memory consumption: {} KB", rss.value());

      if (soft_memory_limit_in_kb.has_value() && rss.value() > soft_memory_limit_in_kb.value()) {
         SPDLOG_INFO("Manually invoking malloc_trim() to give back memory to OS.");
         malloc_trim(0);
      }
   }
}

}  // namespace silo::api

#else

namespace silo::api {

MemoryMonitor::MemoryMonitor(std::optional<uint32_t> soft_memory_limit_in_kb)
    : soft_memory_limit_in_kb(soft_memory_limit_in_kb) {}

void MemoryMonitor::checkRssAndLimit(Poco::Timer& /*timer*/) {}

}  // namespace silo::api

#endif
