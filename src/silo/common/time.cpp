#include "silo/common/time.h"

#include <iomanip>

std::time_t silo::common::mapToTime(const std::string& value) {
   struct std::tm time_struct {};
   std::istringstream time_stream(value);
   time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
   return mktime(&time_struct);
}
