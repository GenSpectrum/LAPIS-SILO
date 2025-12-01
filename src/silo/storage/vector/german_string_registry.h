#pragma once

#include <vector>

#include <boost/serialization/deque.hpp>

#include "silo/common/german_string.h"
#include "silo/common/types.h"
#include "silo/storage/buffer/page.h"

namespace silo::storage::vector {

class GermanStringPage {
  public:
   // The 16KB buffer is structured as follows:
   //
   //     2B     14B          16B
   //    |---|----------|--------------|--------------|--------------|
   //    | n | reserved |  SiloString  |  SiloString  |  SiloString  |
   //    |---|----------|--------------|--------------|--------------|
   //    |  SiloString  |  SiloString  |  SiloString  |              |
   //    |--------------|--------------|--------------|              |
   //    |                                                           |
   //    |                                                           |
   //    |                            ...                            |
   //    |-----------------------------------------------------------|
   //                                                                `16384
   //
   // A total of 16384/16 - 1= 1023 SiloStrings would fit on the page
   static_assert(sizeof(SiloString) == 16);
   static constexpr size_t MAX_STRINGS_PER_PAGE = 1023;
   struct Header {
      std::array<uint8_t, 16> bytes;
   };

  private:
   buffer::Page page;

  public:
   GermanStringPage() { std::memset(page.buffer, 0, sizeof(Header)); }

   [[nodiscard]] uint16_t& n() const { return *reinterpret_cast<uint16_t*>(page.buffer); }

   [[nodiscard]] bool full() const { return n() == MAX_STRINGS_PER_PAGE; }

   [[nodiscard]] size_t insert(const SiloString& silo_string) const {
      SILO_ASSERT(full() == false);
      uint8_t* start_of_next_string_struct =
         page.buffer + sizeof(Header) + (n() * sizeof(SiloString));
      new (start_of_next_string_struct) SiloString(silo_string);
      return n()++;
   }

   const SiloString& get(Idx& row_id) const {
      SILO_ASSERT(row_id < n());
      uint8_t* start_of_string_struct =
         page.buffer + sizeof(Header) + (row_id * sizeof(SiloString));
      return *reinterpret_cast<SiloString*>(start_of_string_struct);
   }

  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & page;
      // clang-format on
   }
};

class GermanStringRegistry {
   std::deque<GermanStringPage> german_string_pages;

  public:
   Idx insert(const SiloString& silo_string);

   [[nodiscard]] SiloString get(Idx row_id) const;

   [[nodiscard]] size_t numValues() const {
      if (german_string_pages.empty()) {
         return 0;
      }
      return ((german_string_pages.size() - 1) * GermanStringPage::MAX_STRINGS_PER_PAGE) +
             german_string_pages.back().n();
   }

  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & german_string_pages;
      // clang-format on
   }
};

}  // namespace silo::storage::vector
