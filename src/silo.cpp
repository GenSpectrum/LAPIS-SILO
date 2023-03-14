//
// Created by Alexander Taepper on 27.09.22.
//

#include <silo/common/istream_wrapper.h>
#include <silo/common/silo_symbols.h>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <syncstream>

silo::istream_wrapper::istream_wrapper(const std::string& file_name) {
   if (file_name.ends_with(".xz")) {
      file = std::ifstream(file_name, std::ios::binary);
      std::unique_ptr<boost::iostreams::filtering_istream> archive =
         std::make_unique<boost::iostreams::filtering_istream>();
      archive->push(boost::iostreams::lzma_decompressor());
      archive->push(file);
      actual_stream = std::move(archive);
   } else {
      actual_stream = make_unique<std::ifstream>(file_name, std::ios::binary);
   }
}
struct separate_thousands : std::numpunct<char> {
   [[nodiscard]] char_type do_thousands_sep() const override { return '\''; }
   [[nodiscard]] string_type do_grouping() const override { return "\3"; }
};

std::string silo::number_fmt(unsigned long n) {
   std::ostringstream oss;
   auto thousands = std::make_unique<separate_thousands>();
   oss.imbue(std::locale(oss.getloc(), thousands.release()));
   oss << n;
   return oss.str();
}

std::string getPangoPrefix(const std::string& pango_lineage) {
   std::string pangoPref;
   if (pango_lineage.size() > 2) {
      std::stringstream ss(pango_lineage);
      if (!getline(ss, pangoPref, '.')) {
         std::cerr << "Non-covered case of pango lineage!" << std::endl;
         return "Not-recognized";
      }
   } else {
      pangoPref = pango_lineage;
   }
   return pangoPref;
}
