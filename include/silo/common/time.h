#ifndef SILO_TIME_H
#define SILO_TIME_H

#include <ctime>
#include <sstream>
#include <string>

namespace silo::common {

std::time_t mapToTime(const std::string& value);

}

#endif  // SILO_TIME_H
