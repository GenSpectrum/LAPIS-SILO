#ifndef SILO_SYMBOLS_H
#define SILO_SYMBOLS_H

#include <cstdint>

namespace silo {

template <typename T>
class Util {
  public:
   static constexpr uint32_t count = 0;

   static constexpr std::array<T, count> symbols{};
};

}  // namespace silo

#endif  // SILO_SYMBOLS_H
