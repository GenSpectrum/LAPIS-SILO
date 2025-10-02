#pragma once

#include <cstddef>
#include <memory>

#include <boost/serialization/binary_object.hpp>

namespace silo::storage::buffer {

const size_t PAGE_SIZE = 16384;

class Page {
  public:
   uint8_t* buffer;

   Page() { buffer = new uint8_t[PAGE_SIZE]; }

   Page(Page&& other) noexcept
       : buffer(other.buffer) {
      other.buffer = nullptr;
   }

   Page& operator=(Page&& other) noexcept {
      std::swap(buffer, other.buffer);
      return *this;
   }

   ~Page() {
      delete[] buffer;
   }

   Page(const Page& other) = delete;
   Page operator=(const Page& other) = delete;

  private:
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      if(Archive::is_saving::value) {
         // Saving: write buffer contents to archive
         archive & boost::serialization::make_binary_object(buffer, PAGE_SIZE);
      } else {
         // Loading: ensure buffer is allocated, then read from archive
         if(!buffer) {
            buffer = reinterpret_cast<uint8_t*>(malloc(PAGE_SIZE));
         }
         archive & boost::serialization::make_binary_object(buffer, PAGE_SIZE);
      }
      // clang-format on
   }
};

}  // namespace silo::storage::buffer
