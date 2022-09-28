#ifndef SILO_H
#define SILO_H

#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <utility>
#include <vector>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "roaring/roaring.hh"
#include "util.h"

using namespace std;

namespace silo {

   static constexpr unsigned genomeLength = 29903;

// https://www.bioinformatics.org/sms/iupac.html
   enum Symbol {
      gap, // . or -, gap
      A, // Adenine
      C, // Cytosine
      G, // Guanine
      T, // (or U) Thymine (or Uracil)
      R, // A or G
      Y, // C or T
      S, // G or C
      W, // A or T
      K, // G or T
      M, // A or C
      B, // C or G or T
      D, // A or G or T
      H, // A or C or T
      V, // A or C or G
      N, // any base
   };

   enum Residue {
      aA,
      aC,
      aG,
      aT
   };

   static constexpr unsigned symbolCount = static_cast<unsigned>(Symbol::N) + 1;

   static constexpr char symbol_rep[symbolCount] = {
           '-', 'A', 'C', 'G', 'T', 'R', 'Y', 'S',
           'W', 'K', 'M', 'B', 'D', 'H', 'V', 'N'};

   static_assert(symbol_rep[static_cast<unsigned>(Symbol::N)] == 'N');

   Symbol to_symbol(char c);

   std::string getPangoPrefix(const std::string &pango_lineage);


   struct separate_thousands : std::numpunct<char> {
      [[nodiscard]] char_type do_thousands_sep() const override { return '\''; }
      [[nodiscard]] string_type do_grouping() const override { return "\3"; }
   };

   static inline std::string number_fmt(unsigned long n)
   {
      std::ostringstream oss;
      auto thousands = std::make_unique<separate_thousands>();
      oss.imbue(std::locale(oss.getloc(), thousands.release()));
      oss << n;
      return oss.str();
   }

   struct istream_wrapper {
      ifstream file;

      unique_ptr<istream> actual_stream;

      explicit istream_wrapper(const string& file_name){
         if(file_name.ends_with(".xz")){
            file = ifstream(file_name, ios::binary);
            unique_ptr<boost::iostreams::filtering_istream> archive = make_unique<boost::iostreams::filtering_istream>();
            archive->push(boost::iostreams::lzma_decompressor());
            archive->push(file);
            actual_stream = std::move(archive);
         }
         else {
            actual_stream = make_unique<ifstream>(file_name, ios::binary);
         }
      }

      istream& get_is() const{
         return *actual_stream;
      }
   };

} // namespace silo;

BOOST_SERIALIZATION_SPLIT_FREE(::roaring::Roaring)
namespace boost::serialization {

   template<class Archive>
   [[maybe_unused]] void save(Archive &ar, const roaring::Roaring &bitmask,
                              [[maybe_unused]] const unsigned int version) {
      std::size_t expected_size_in_bytes = bitmask.getSizeInBytes();
      std::vector<char> buffer(expected_size_in_bytes);
      std::size_t size_in_bytes = bitmask.write(buffer.data());

      ar & size_in_bytes;
      ar & ::boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
   }

   template<class Archive>
   [[maybe_unused]] void load(Archive &ar, roaring::Roaring &bitmask, [[maybe_unused]] const unsigned int version) {
      std::size_t size_in_bytes = 0;
      ar & size_in_bytes;
      std::vector<char> buffer(size_in_bytes);
      ar & ::boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
      bitmask = roaring::Roaring::readSafe(buffer.data(), size_in_bytes);
   }
}  // namespace boost

#endif //SILO_H