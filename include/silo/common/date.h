#ifndef SILO_DATE_H
#define SILO_DATE_H

#include <string>

namespace silo::common {

typedef uint32_t Date;

silo::common::Date stringToDate(const std::string& value);

std::string dateToString(silo::common::Date date);

}  // namespace silo::common

#endif  // SILO_DATE_H
