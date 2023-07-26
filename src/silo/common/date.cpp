#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include "silo/common/date.h"
#include "silo/common/date_format_exception.h"

namespace {

constexpr uint32_t NUMBER_OF_MONTHS = 12;
constexpr uint32_t NUMBER_OF_DAYS = 31;
constexpr uint32_t BYTES_FOR_MONTHS = 4;
constexpr uint32_t BYTES_FOR_DAYS = 12;

}  // namespace

silo::common::Date silo::common::stringToDate(const std::string& value) {
   if (value.empty()) {
      return 0;
   }
   auto split_position = value.find('-', 0);
   if (split_position == std::string::npos) {
      SPDLOG_INFO("Expect dates to be delimited by '-': " + value + "\nIgnoring date");
      return 0;
   }
   auto split_position2 = value.find('-', split_position + 1);
   if (split_position2 == std::string::npos) {
      SPDLOG_INFO("Expect dates to be delimited twice by '-': " + value + "\nIgnoring date");
      return 0;
   }
   const std::string year_string = value.substr(0, split_position);
   const std::string month_string = value.substr(split_position + 1, split_position2);
   const std::string day_string = value.substr(split_position2 + 1);
   try {
      const uint32_t year = stoi(year_string);
      const uint32_t month = stoi(month_string);
      const uint32_t day = stoi(day_string);
      if (month > NUMBER_OF_MONTHS || month == 0) {
         SPDLOG_INFO("Month is not in [1,12] " + value + "\nIgnoring date");
         return 0;
      }
      if (day > NUMBER_OF_DAYS || day == 0) {
         SPDLOG_INFO("Day is not in [1,31] " + value + "\nIgnoring date");
         return 0;
      }
      // Date is stored with the year in the upper 16 bits, month in bits [12,16), and day [0,12)
      const uint32_t date_value =
         (year << (BYTES_FOR_MONTHS + BYTES_FOR_DAYS)) + (month << BYTES_FOR_DAYS) + day;
      return Date{date_value};
   } catch (const std::invalid_argument& ex) {
      SPDLOG_INFO(
         "Parsing of date failed: " + value + "\nWith exception: " + ex.what() + "\nIgnoring date"
      );
      return 0;
   } catch (const std::out_of_range& ex) {
      SPDLOG_INFO(
         "Parsing of date failed: " + value + "\nWith exception: " + ex.what() + "\nIgnoring date"
      );
      return 0;
   }
}

std::optional<std::string> silo::common::dateToString(silo::common::Date date) {
   if (date == 0) {
      return std::nullopt;
   }
   // Date is stored with the year in the upper 16 bits, month in bits [12,16), and day [0,12)
   const uint32_t year = date >> (BYTES_FOR_MONTHS + BYTES_FOR_DAYS);
   const uint32_t month = (date >> BYTES_FOR_DAYS) & 0xF;
   const uint32_t day = date & 0xFFF;

   std::ostringstream result_string;
   result_string << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-"
                 << std::setw(2) << day;

   return result_string.str();
}
