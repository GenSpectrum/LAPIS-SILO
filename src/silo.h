#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "roaring/roaring.hh"
#include "roaring/roaring.c"
#include "util.h"

using namespace std;

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

enum Residue{
   aA,
   aC,
   aG,
   aT
};

static constexpr unsigned symbolCount = static_cast<unsigned>(Symbol::N) + 1;

static constexpr char symbol_rep[symbolCount] = {
        '-', 'A', 'C', 'G', 'T', 'R', 'Y', 'S',
        'W', 'K', 'M', 'B', 'D', 'H', 'V', 'N'};


Symbol to_symbol(char c){
   Symbol s = Symbol::gap;
   switch (c) {
      case '.':
      case '-': s = Symbol::gap; break;
      case 'A': s = Symbol::A; break;
      case 'C': s = Symbol::C; break;
      case 'G': s = Symbol::G; break;
      case 'T':
      case 'U': s = Symbol::T; break;
      case 'R': s = Symbol::R; break;
      case 'Y': s = Symbol::Y; break;
      case 'S': s = Symbol::S; break;
      case 'W': s = Symbol::W; break;
      case 'K': s = Symbol::K; break;
      case 'M': s = Symbol::M; break;
      case 'B': s = Symbol::B; break;
      case 'D': s = Symbol::D; break;
      case 'H': s = Symbol::H; break;
      case 'V': s = Symbol::V; break;
      case 'N': s = Symbol::N; break;
      default: cerr << "unrecognized symbol " << c << endl;
   }
   return s;
}

static_assert(symbol_rep[static_cast<unsigned>(Symbol::N)] == 'N');

BOOST_SERIALIZATION_SPLIT_FREE(roaring::Roaring)
namespace boost::serialization {

   template <class Archive>
   [[maybe_unused]] void save(Archive& ar, const roaring::Roaring& bitmask,
                              [[maybe_unused]] const unsigned int version) {
      std::size_t expected_size_in_bytes = bitmask.getSizeInBytes();
      std::vector<char> buffer(expected_size_in_bytes);
      std::size_t       size_in_bytes = bitmask.write(buffer.data());

      ar& size_in_bytes;
      ar& boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
   }

   template <class Archive>
   void load(Archive& ar, roaring::Roaring& bitmask, [[maybe_unused]] const unsigned int version) {
      std::size_t size_in_bytes = 0;
      ar& size_in_bytes;
      std::vector<char> buffer(size_in_bytes);
      ar&  boost::serialization::make_binary_object(buffer.data(), size_in_bytes);
      bitmask = roaring::Roaring::readSafe(buffer.data(), size_in_bytes);
   }
}  // namespace boost
