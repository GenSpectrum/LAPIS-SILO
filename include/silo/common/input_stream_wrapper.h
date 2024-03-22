#pragma once

#include <filesystem>
#include <fstream>
#include <memory>

#include <boost/iostreams/filtering_stream.hpp>

namespace silo {
class InputStreamWrapper {
  private:
   std::ifstream file;
   std::unique_ptr<boost::iostreams::filtering_istream> input_stream;

  public:
   explicit InputStreamWrapper(const std::filesystem::path& filename);

   [[nodiscard]] std::istream& getInputStream() const;
};
}  // namespace silo
