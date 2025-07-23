#pragma once

#include <iterator>
#include <ranges>

#include <simdjson.h>

#include "evobench/evobench.hpp"
#include "silo/append/append_exception.h"

namespace silo::append {

class NdjsonLineReader {
   std::istream* input_stream;
   std::string line_buffer;
   simdjson::ondemand::parser parser;
   simdjson::ondemand::document json_document_buffer;
   simdjson::error_code error;

  public:
   class iterator {
     public:
      using value_type =
         std::pair<simdjson::simdjson_result<simdjson::ondemand::document>, std::string_view>;
      using reference = std::
         pair<simdjson::simdjson_result<simdjson::ondemand::document_reference>, std::string_view>;
      using pointer = void;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::input_iterator_tag;

      iterator()
          : stream(nullptr),
            at_end(true) {}

      iterator(NdjsonLineReader* stream)
          : stream(stream),
            at_end(false) {
         ++(*this);  // prime first json
      }

      reference operator*() {
         return {
            simdjson::simdjson_result<simdjson::ondemand::document_reference>(
               stream->json_document_buffer, stream->error
            ),
            stream->line_buffer
         };
      }

      iterator& operator++() {
         EVOBENCH_SCOPE("NdjsonLineReader::Iterator", "operator++");
         if (stream->error) {
            at_end = true;
         }
         // if stream->next() ever returns true, it will not be read from again
         if (stream->next()) {
            at_end = true;
         }
         return *this;
      }

      iterator operator++(int) {
         iterator tmp = *this;
         ++(*this);
         return tmp;
      }

      bool operator==(const iterator& other) const { return at_end && other.at_end; }

      bool operator!=(const iterator& other) const { return !(*this == other); }

     private:
      // The stream in which the iterator is operating
      NdjsonLineReader* stream;
      bool at_end = false;
   };

   explicit NdjsonLineReader(std::istream& input_stream)
       : input_stream(&input_stream),
         line_buffer(),
         parser(),
         json_document_buffer(),
         error(){};

   [[nodiscard]] iterator begin() { return iterator(this); }
   [[nodiscard]] iterator end() const { return iterator(); }

  private:
   bool next() {
      if (error) {
         return true;
      }
      while (true) {
         // We need this check here, because we only check for 'eof && empty'
         // after we read to also allow files that do not end with a line-break
         if (input_stream->eof()) {
            return true;
         }
         std::getline(*input_stream, line_buffer);

         if (input_stream->eof() && line_buffer.empty()) {
            return true;
         }
         if (input_stream->fail()) {
            error = simdjson::IO_ERROR;
            return true;
         }

         error = parser.iterate(line_buffer).get(json_document_buffer);
         if (error == simdjson::EMPTY) {
            continue;
         }
         return false;
      }
   }
};

}  // namespace silo::append
