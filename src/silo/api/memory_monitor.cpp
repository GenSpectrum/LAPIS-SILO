#include "silo/api/memory_monitor.h"

#if defined(__linux__)

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include <re2/re2.h>
#include <spdlog/spdlog.h>

#include "silo/common/allocator.h"

namespace {

std::optional<uint32_t> parseVmRSSLine(const std::string& line) {
   static const re2::RE2 vm_rss_regex("VmRSS:\\s*(\\d+) kB");
   std::string match;

   if (re2::RE2::PartialMatch(line, vm_rss_regex, &match)) {
      try {
         return std::stol(match);
      } catch (const std::out_of_range& oor) {
         SPDLOG_DEBUG(
            "parseVmRSSLine: VmRSS value out of range for long: {} - {}", match, oor.what()
         );
         return std::nullopt;
      } catch (const std::invalid_argument& ia) {
         SPDLOG_DEBUG("parseVmRSSLine: Invalid argument for stol: {} - {}", match, ia.what());
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
      if (line.starts_with("VmRSS:")) {
         return parseVmRSSLine(line);
      }
   }

   SPDLOG_DEBUG("getResidentSetSize: VmRSS line not found in {}", path.string());
   return std::nullopt;
}

const int64_t FIVE_SECONDS = 5000;

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
         silo::common::Allocator::trim();
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
