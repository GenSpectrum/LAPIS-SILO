#include "silo/common/input_stream_wrapper.h"

#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace silo {
InputStreamWrapper::InputStreamWrapper(const std::string& filename) {
   if (filename.ends_with(".xz")) {
      file = std::ifstream(filename, std::ios::binary);
      std::unique_ptr<boost::iostreams::filtering_istream> archive =
         std::make_unique<boost::iostreams::filtering_istream>();
      archive->push(boost::iostreams::lzma_decompressor());
      archive->push(file);
      input_stream = std::move(archive);
   } else {
      input_stream = make_unique<std::ifstream>(filename, std::ios::binary);
   }
}

std::istream& silo::InputStreamWrapper::getInputStream() const {
   return *input_stream;
}
}  // namespace silo
