#pragma once

#include <string_view>

namespace rhydb {

#ifdef RHYDB_RELEASE_VERSION
constexpr std::string_view RELEASE_VERSION = RHYDB_RELEASE_VERSION;
#else
constexpr std::string_view RELEASE_VERSION = "local";
#endif

}  // namespace rhydb
