#pragma once

#include <string>

#include <fmt/format.h>
#include <boost/lexical_cast.hpp>

class Ignored {
  public:
   Ignored() = default;
};

namespace boost {
template <>
inline Ignored lexical_cast<Ignored, std::string>(const std::string& /*unused*/) {
   return {};
}
}  // namespace boost

template <>
struct [[maybe_unused]] fmt::formatter<Ignored> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(const Ignored& val, fmt::format_context& ctx)
      -> decltype(ctx.out());
};
