#ifndef SILO_ISTREAM_WRAPPER_H
#define SILO_ISTREAM_WRAPPER_H

#include <fstream>
#include <iostream>
#include <memory>

namespace silo {
struct InputStreamWrapper {
  private:
   std::ifstream file;
   std::unique_ptr<std::istream> input_stream;

  public:
   explicit InputStreamWrapper(const std::string& filename);

   std::istream& getInputStream() const;
};
}  // namespace silo

#endif  // SILO_ISTREAM_WRAPPER_H