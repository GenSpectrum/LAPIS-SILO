//
// Created by Alexander Taepper on 09.01.23.
//

#ifndef SILO_ISTREAM_WRAPPER_H
#define SILO_ISTREAM_WRAPPER_H

#include <fstream>
#include <iostream>
#include <memory>

namespace silo {
struct istream_wrapper {
   private:
   std::ifstream file;
   std::unique_ptr<std::istream> actual_stream;

   public:
   explicit istream_wrapper(const std::string& file_name);

   std::istream& get_is() const {
      return *actual_stream;
   }
};
}

#endif //SILO_ISTREAM_WRAPPER_H
