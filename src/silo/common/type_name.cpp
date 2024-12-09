#include "silo/common/type_name.h"

#include <filesystem>

// Specializations

namespace silo::common {

template <>
std::string typeName<std::string>() {
   return "string";
}

template <>
std::string typeName<std::filesystem::path>() {
   return "path";
}

template <>
std::string typeName<bool>() {
   return "bool";
}

template <>
std::string typeName<uint32_t>() {
   return "u32";
}
template <>
std::string typeName<int32_t>() {
   return "i32";
}
template <>
std::string typeName<uint64_t>() {
   return "u64";
}
template <>
std::string typeName<int64_t>() {
   return "i64";
}
template <>
std::string typeName<uint16_t>() {
   return "u16";
}
template <>
std::string typeName<int16_t>() {
   return "i16";
}
template <>
std::string typeName<uint8_t>() {
   return "u8";
}
template <>
std::string typeName<int8_t>() {
   return "i8";
}

}  // namespace silo::common
