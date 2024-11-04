#include "silo/common/fmt_formatters.h"

namespace silo::common {

std::string toIsoString(
   const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time_point
) {
   // Convert the time_point to system time (std::time_t)
   std::time_t time = std::chrono::system_clock::to_time_t(time_point);

   // Get the nanoseconds part
   auto nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(time_point.time_since_epoch()) %
      1'000'000'000;

   // Convert to UTC time (std::tm)
   std::tm utime = *std::gmtime(&time);

   // Create an ISO 8601 string with nanoseconds precision
   std::ostringstream oss;
   oss << std::put_time(&utime, "%Y-%m-%dT%H:%M:%S");
   oss << '.' << std::setfill('0') << std::setw(9) << nanoseconds.count()
       << 'Z';  // Appending 'Z' for UTC time

   return oss.str();
}
}  // namespace silo::common
