#include "config/config_key_path.h"

#include <boost/algorithm/string/join.hpp>

std::string ConfigKeyPath::toDebugString() const {
   return boost::join(path, "/");
}
