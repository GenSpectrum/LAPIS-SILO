#ifndef SILO_TYPES_H
#define SILO_TYPES_H

#include <stddef.h>
#include <cstdint>

namespace silo {
// Referencing to silo internal pointers
typedef uint32_t Idx;
// Representing the ID of a row in the database
typedef size_t TID;
}  // namespace silo

#endif  // SILO_TYPES_H
