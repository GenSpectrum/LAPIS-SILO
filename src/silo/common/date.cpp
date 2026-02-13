#include <charconv>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>

#include "silo/common/date.h"

namespace {

constexpr uint32_t NUMBER_OF_MONTHS = 12;
constexpr uint32_t NUMBER_OF_DAYS = 31;
constexpr uint32_t BYTES_FOR_MONTHS = 4;
constexpr uint32_t BYTES_FOR_DAYS = 12;

}  // namespace

silo::common::Date silo::common::stringToDate(std::string_view value) {
   if (value.empty()) {
      return NULL_DATE;
   }
   auto split_position = value.find('-', 0);
   if (split_position == std::string::npos) {
      SPDLOG_WARN("Expect dates to be delimited by '-': {}\nIgnoring date", value);
      return NULL_DATE;
   }
   auto split_position2 = value.find('-', split_position + 1);
   if (split_position2 == std::string::npos) {
      SPDLOG_WARN("Expect dates to be delimited twice by '-': {}\nIgnoring date", value);
      return NULL_DATE;
   }
   const std::string_view year_string = value.substr(0, split_position);
   const std::string_view month_string = value.substr(split_position + 1, split_position2);
   const std::string_view day_string = value.substr(split_position2 + 1);
   uint32_t year;
   if (std::from_chars(year_string.data(), year_string.data() + year_string.size(), year).ec !=
       std::errc{}) {
      SPDLOG_WARN("Parsing of year failed: {}. Ignoring date", year_string);
      return NULL_DATE;
   }
   uint32_t month;
   if (std::from_chars(month_string.data(), month_string.data() + month_string.size(), month).ec !=
       std::errc{}) {
      SPDLOG_WARN("Parsing of month failed: {}. Ignoring date", month_string);
      return NULL_DATE;
   }
   uint32_t day;
   if (std::from_chars(day_string.data(), day_string.data() + day_string.size(), day).ec !=
       std::errc{}) {
      SPDLOG_WARN("Parsing of day failed: {}. Ignoring date", day_string);
      return NULL_DATE;
   }
   if (month > NUMBER_OF_MONTHS || month == 0) {
      SPDLOG_WARN("Month is not in [1,{}]: {} \nIgnoring date", NUMBER_OF_MONTHS, value);
      return NULL_DATE;
   }
   if (day > NUMBER_OF_DAYS || day == 0) {
      SPDLOG_WARN("Month is not in [1,{}]: {} \nIgnoring date", NUMBER_OF_DAYS, value);
      return NULL_DATE;
   }
   // Date is stored with the year in the upper 16 bits, month in bits [12,16), and day [0,12)
   const uint32_t date_value =
      (year << (BYTES_FOR_MONTHS + BYTES_FOR_DAYS)) + (month << BYTES_FOR_DAYS) + day;
   return Date{date_value};
}

std::optional<std::string> silo::common::dateToString(silo::common::Date date) {
   if (date == 0) {
      return std::nullopt;
   }
   // Date is stored with the year in the upper 16 bits, month in bits [12,16), and day [0,12)
   const uint32_t year = date >> (BYTES_FOR_MONTHS + BYTES_FOR_DAYS);
   const uint32_t month = (date >> BYTES_FOR_DAYS) & 0xF;
   const uint32_t day = date & 0xFFF;

   return fmt::format("{:04}-{:02}-{:02}", year, month, day);
}
