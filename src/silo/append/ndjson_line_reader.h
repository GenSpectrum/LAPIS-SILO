#pragma once

#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/append/append_exception.h"

namespace silo::append {

class NdjsonLineReader {
  public:
   class Iterator {
     public:
      using value_type = nlohmann::json;
      using reference = const nlohmann::json&;
      using pointer = const nlohmann::json*;
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;

      Iterator()
          : stream_(nullptr),
            at_end(true) {}

      Iterator(std::istream* stream)
          : stream_(stream),
            line_number(0),
            at_end(false) {
         ++(*this);  // prime first json
      }

      reference operator*() const { return json_buffer; }
      pointer operator->() const { return &json_buffer; }

      Iterator& operator++() {
         EVOBENCH_SCOPE("NdjsonLineReader::Iterator", "operator++");
         std::string line;
         while (std::getline(*stream_, line)) {
            if (line.empty())
               continue;
            try {
               json_buffer = nlohmann::json::parse(line);
            } catch (const nlohmann::json::parse_error& parse_error) {
               throw silo::append::AppendException(
                  "Error while parsing ndjson file: {}", parse_error.what()
               );
            }
            ++line_number;
            return *this;
         }
         at_end = true;
         stream_ = nullptr;
         return *this;
      }

      Iterator operator++(int) {
         Iterator tmp = *this;
         ++(*this);
         return tmp;
      }

      bool operator==(const Iterator& other) const {
         return (at_end && other.at_end) || (stream_ == other.stream_);
      }

      bool operator!=(const Iterator& other) const { return !(*this == other); }

     private:
      std::istream* stream_;
      nlohmann::json json_buffer;
      size_t line_number = 0;
      bool at_end = false;
   };

   explicit NdjsonLineReader(std::istream& stream)
       : stream_(&stream) {}

   [[nodiscard]] Iterator begin() const { return Iterator(stream_); }
   [[nodiscard]] Iterator end() const { return Iterator(); }

  private:
   std::istream* stream_;
};

}  // namespace silo::append
