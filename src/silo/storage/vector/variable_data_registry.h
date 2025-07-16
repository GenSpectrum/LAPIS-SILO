#pragma once

#include <cstddef>
#include <deque>
#include <vector>

#include <boost/serialization/deque.hpp>

#include "silo/storage/buffer/page.h"

namespace silo::storage::vector {

class VariableDataRegistry {
   std::deque<buffer::Page> variable_data_pages;
   uint16_t offset;

  public:
   struct DataList {
      std::string_view data;
      std::unique_ptr<DataList> continuation;
   };

   struct Identifier {
      uint32_t page_id;
      // We only need 14 bits to index into our 16KB pages, but we may use the additional 2 bits for
      // some flags later on
      uint16_t offset;
   };
   static_assert(sizeof(Identifier) == 8);

   VariableDataRegistry::Identifier insert(std::string_view data);

   DataList get(VariableDataRegistry::Identifier id) const;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & variable_data_pages;
      archive & offset;
      // clang-format on
   }
};
}  // namespace silo::storage::vector
