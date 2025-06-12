#pragma once

#include <string_view>

namespace silo {

#ifdef SILO_RELEASE_VERSION
constexpr std::string_view RELEASE_VERSION = SILO_RELEASE_VERSION;
#else
constexpr std::string_view RELEASE_VERSION = "local";
#endif

}  // namespace silo
