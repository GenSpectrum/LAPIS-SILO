#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>

#include <boost/iostreams/filtering_stream.hpp>

namespace silo {
class InputStreamWrapper {
  private:
   std::ifstream file_stream;
   std::unique_ptr<std::istream> input_stream;

  public:
   explicit InputStreamWrapper(const std::filesystem::path& file_path);
   explicit InputStreamWrapper(const std::string& content);
   explicit InputStreamWrapper(std::unique_ptr<std::istream> existing_stream)
       : input_stream(std::move(existing_stream)) {}

   [[nodiscard]] std::istream& getInputStream() const;

   static InputStreamWrapper openFileOrStdIn(
      const std::optional<std::filesystem::path>& maybe_file_path
   );
};
}  // namespace silo
