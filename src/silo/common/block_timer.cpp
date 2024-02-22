#include "silo/common/block_timer.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace silo::common {

std::string formatDuration(int64_t int_microseconds) {
   auto microseconds = std::chrono::microseconds(int_microseconds);
   auto hours = std::chrono::duration_cast<std::chrono::hours>(microseconds);
   microseconds -= hours;
   auto minutes = std::chrono::duration_cast<std::chrono::minutes>(microseconds);
   microseconds -= minutes;
   auto seconds = std::chrono::duration_cast<std::chrono::seconds>(microseconds);
   microseconds -= seconds;
   auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(microseconds);

   std::stringstream string_stream;

   string_stream << std::setw(2) << std::setfill('0') << hours.count() << ":";
   string_stream << std::setw(2) << std::setfill('0') << minutes.count() << ":";
   string_stream << std::setw(2) << std::setfill('0') << seconds.count() << ".";
   string_stream << std::setw(3) << std::setfill('0') << milliseconds.count();

   return string_stream.str();
}

}  // namespace silo::common
