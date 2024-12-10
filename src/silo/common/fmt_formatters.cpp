#include "silo/common/fmt_formatters.h"

namespace silo::common {

std::string toIsoString(
   const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& time_point
) {
   auto duration_since_epoch = time_point.time_since_epoch();

   auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch);
   const std::time_t time = seconds_since_epoch.count();

   auto nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch) % 1'000'000'000;

   const std::tm utime = *std::gmtime(&time);

   // Create an ISO 8601 string with nanoseconds precision
   std::ostringstream oss;
   oss << std::put_time(&utime, "%Y-%m-%dT%H:%M:%S");
   oss << '.' << std::setfill('0') << std::setw(9) << nanoseconds.count()
       << 'Z';  // Appending 'Z' for UTC time

   return oss.str();
}
}  // namespace silo::common
