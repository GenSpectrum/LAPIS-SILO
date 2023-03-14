#ifndef SILO_VARIANT_JSON_SERIALIZER_H
#define SILO_VARIANT_JSON_SERIALIZER_H

#include <nlohmann/json.hpp>
#include <variant>

namespace nlohmann {
template <typename T, typename... Ts>
void variant_from_json(const nlohmann::json& j, std::variant<Ts...>& data) {
   try {
      data = j.get<T>();
   } catch (...) {
   }
}

template <typename... Ts>
struct adl_serializer<std::variant<Ts...>> {
   static void to_json(nlohmann::json& j, const std::variant<Ts...>& data) {
      std::visit([&j](const auto& v) { j = v; }, data);
   }

   static void from_json(const nlohmann::json& j, std::variant<Ts...>& data) {
      (variant_from_json<Ts>(j, data), ...);
   }
};
}  // namespace nlohmann

#endif  // SILO_VARIANT_JSON_SERIALIZER_H
