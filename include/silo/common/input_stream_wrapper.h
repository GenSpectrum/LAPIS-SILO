#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>

#include <boost/iostreams/filtering_stream.hpp>

namespace silo {
class InputStreamWrapper {
  private:
   std::ifstream file_stream;
   std::istringstream string_stream;
   std::unique_ptr<boost::iostreams::filtering_istream> input_stream;

  public:
   explicit InputStreamWrapper(const std::filesystem::path& filename);
   explicit InputStreamWrapper(const std::string& content);

   [[nodiscard]] std::istream& getInputStream() const;
};
}  // namespace silo
