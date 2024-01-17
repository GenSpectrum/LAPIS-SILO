#include "silo/common/input_stream_wrapper.h"

#include <string>

#include <spdlog/spdlog.h>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace {

std::filesystem::path withXZending(const std::filesystem::path& filename) {
   return {filename.extension() == ".xz" ? filename.string() : filename.string() + ".xz"};
}

std::filesystem::path withZSTending(const std::filesystem::path& filename) {
   return {filename.extension() == ".zst" ? filename.string() : filename.string() + ".zst"};
}

}  // namespace

namespace silo {
InputStreamWrapper::InputStreamWrapper(const std::filesystem::path& filename)
    : input_stream(std::make_unique<boost::iostreams::filtering_istream>()) {
   if (std::filesystem::is_regular_file(withZSTending(filename))) {
      SPDLOG_INFO("Detected file-ending .zst for input file " + filename.string());
      file = std::ifstream(withZSTending(filename), std::ios::binary);
      input_stream->push(boost::iostreams::zstd_decompressor());
   } else if (std::filesystem::is_regular_file(withXZending(filename))) {
      SPDLOG_INFO("Detected file-ending .xz for input file " + filename.string());
      file = std::ifstream(withXZending(filename), std::ios::binary);
      input_stream->push(boost::iostreams::lzma_decompressor());
   } else if (std::filesystem::is_regular_file(filename)) {
      SPDLOG_INFO("Detected file without specialized ending, processing raw: " + filename.string());
      file = std::ifstream(filename, std::ios::binary);
   } else {
      throw silo::preprocessing::PreprocessingException(
         "Cannot find file with name or associated endings (.xz, .zst): " + filename.string()
      );
   }
   input_stream->push(file);
}

std::istream& silo::InputStreamWrapper::getInputStream() const {
   return *input_stream;
}

}  // namespace silo
