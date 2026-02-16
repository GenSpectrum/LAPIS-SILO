#include "silo/common/input_stream_wrapper.h"

#include <fstream>
#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace {

std::filesystem::path withXZending(const std::filesystem::path& file_path) {
   return {file_path.extension() == ".xz" ? file_path.string() : file_path.string() + ".xz"};
}

std::filesystem::path withZSTending(const std::filesystem::path& file_path) {
   return {file_path.extension() == ".zst" ? file_path.string() : file_path.string() + ".zst"};
}

}  // namespace

namespace silo {
InputStreamWrapper::InputStreamWrapper(const std::filesystem::path& file_path) {
   auto boost_input_stream = std::make_unique<boost::iostreams::filtering_istream>();
   if (std::filesystem::is_regular_file(withZSTending(file_path))) {
      SPDLOG_INFO("Detected file-ending .zst for input file " + file_path.string());
      file_stream = std::ifstream(withZSTending(file_path), std::ios::binary);
      boost_input_stream->push(boost::iostreams::zstd_decompressor());
   } else if (std::filesystem::is_regular_file(withXZending(file_path))) {
      SPDLOG_INFO("Detected file-ending .xz for input file " + file_path.string());
      file_stream = std::ifstream(withXZending(file_path), std::ios::binary);
      boost_input_stream->push(boost::iostreams::lzma_decompressor());
   } else if (std::filesystem::is_regular_file(file_path)) {
      SPDLOG_INFO(
         "Detected file without specialized ending, processing raw: " + file_path.string()
      );
      file_stream = std::ifstream(file_path, std::ios::binary);
   } else {
      throw silo::preprocessing::PreprocessingException(
         "Cannot find file with name or associated endings (.xz, .zst): " + file_path.string()
      );
   }
   boost_input_stream->push(file_stream);
   input_stream = std::move(boost_input_stream);
}

InputStreamWrapper::InputStreamWrapper(const std::string& content) {
   input_stream = std::make_unique<std::istringstream>(content);
}

std::istream& silo::InputStreamWrapper::getInputStream() const {
   return *input_stream;
}

InputStreamWrapper InputStreamWrapper::openFileOrStdIn(
   const std::optional<std::filesystem::path>& maybe_file_path
) {
   if (maybe_file_path.has_value()) {
      SPDLOG_DEBUG("Given input file: {}", maybe_file_path.value().string());
      return InputStreamWrapper{maybe_file_path.value()};
   }
   SPDLOG_DEBUG("No input file given, instead opening stdin");
   return InputStreamWrapper{std::make_unique<std::istream>(std::cin.rdbuf())};
}

}  // namespace silo
