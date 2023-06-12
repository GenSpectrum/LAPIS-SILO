#include "silo/common/date.h"

#include "silo/common/date_format_exception.h"

silo::common::Date silo::common::stringToDate(const std::string& value) {
   auto split_position = value.find('-', 0);
   if (split_position == value.size()) {
      throw DateFormatException("Expect dates to be delimited by '-': " + value);
   }
   auto split_position2 = value.find('-', split_position + 1);
   if (split_position2 == value.size()) {
      throw DateFormatException("Expect dates to be delimited by '-': " + value);
   }
   const std::string year_string = value.substr(0, split_position);
   const std::string month_string = value.substr(split_position + 1, split_position2);
   const std::string day_string = value.substr(split_position2 + 1);
   try {
      const uint32_t year = stoi(year_string);
      const uint32_t month = stoi(month_string);
      const uint32_t day = stoi(day_string);
      if (month > 12 || month == 0) {
         throw DateFormatException("Month is not in [1,12] " + value);
      }
      if (day > 31 || day == 0) {
         throw DateFormatException("Day is not in [1,31] " + value);
      }
      const uint32_t date_value = (year << 16) + (month << 12) + day;
      return Date{date_value};
   } catch (const std::invalid_argument& ex) {
      throw DateFormatException(std::string("Parsing of date failed: ") + ex.what());
   } catch (const std::out_of_range& ex) {
      throw DateFormatException(std::string("Parsing of date failed: ") + ex.what());
   }
}

std::string silo::common::dateToString(silo::common::Date date) {
   const uint32_t year = date >> 16;
   const uint32_t month = (date >> 12) & 0xF;
   const uint32_t day = date & 0xFFF;
   return std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day);
}
