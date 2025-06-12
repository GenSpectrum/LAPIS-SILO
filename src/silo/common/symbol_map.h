#pragma once

#include <vector>

#include <boost/serialization/access.hpp>

namespace silo {

template <typename SymbolType, typename T>
class SymbolMap {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & data;
      // clang-format on
   }

   std::array<T, SymbolType::COUNT> data;

  public:
   SymbolMap() {}

   SymbolMap(std::array<T, SymbolType::COUNT>&& data)
       : data(data) {}

   T& operator[](typename SymbolType::Symbol symbol) {
      return data.at(static_cast<uint8_t>(symbol));
   }

   const T& at(typename SymbolType::Symbol symbol) const {
      return data.at(static_cast<uint8_t>(symbol));
   }
};

}  // namespace silo
