#pragma once

#include <string_view>

namespace silo {

#ifdef SILO_VERSION
constexpr std::string_view VERSION = SILO_VERSION;
#else
constexpr std::string_view VERSION = "unknown";
#endif

}  // namespace silo
