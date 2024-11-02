#pragma once

//! A function to get the name of a type (since `typeid(T).name()`
//! doesn't give anything useful).

#include <string>
#include <typeinfo>
#include <unordered_map>

namespace silo::common {

// Template to get the type name
template <typename T>
std::string typeName();

}  // namespace silo::common
