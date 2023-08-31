#include "silo/storage/sequence_store.h"

#include <fstream>
#include <iostream>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/array.hpp>

#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/serialize_optional.h"

void serializeToFile(
   const std::string& filename,
   const silo::Position<silo::Nucleotide>& position
) {
   std::ofstream output_file(filename.c_str(), std::ios::binary);
   ::boost::archive::binary_oarchive output_archive(output_file);
   output_archive << position;
   output_file.close();
}

void deserializeFromFile(const std::string& filename, silo::Position<silo::Nucleotide>& position) {
   std::ifstream input_file(filename, std::ios::binary);
   ::boost::archive::binary_iarchive input_archive(input_file);
   input_archive >> position;
   input_file.close();
}

TEST(Position, shouldSerializeAndDeserializePositionsWithEmptyOptional) {
   const std::string test_file = "test.bin";

   const silo::Position<silo::Nucleotide> position_with_unset_optional(std::nullopt);
   serializeToFile(test_file, position_with_unset_optional);

   silo::Position<silo::Nucleotide> deserialized_position(std::nullopt);
   deserializeFromFile(test_file, deserialized_position);

   EXPECT_FALSE(position_with_unset_optional.symbol_whose_bitmap_is_flipped.has_value());
   EXPECT_FALSE(deserialized_position.symbol_whose_bitmap_is_flipped.has_value());

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}

TEST(Position, shouldSerializeAndDeserializePositionWithSetOptional) {
   const std::string test_file = "test.bin";

   silo::Position<silo::Nucleotide> position_with_set_optional(std::nullopt);
   position_with_set_optional.symbol_whose_bitmap_is_flipped = silo::Nucleotide::Symbol::A;
   serializeToFile(test_file, position_with_set_optional);

   silo::Position<silo::Nucleotide> deserialized_position(std::nullopt);
   deserializeFromFile(test_file, deserialized_position);

   EXPECT_TRUE(deserialized_position.symbol_whose_bitmap_is_flipped.has_value());
   ASSERT_EQ(
      position_with_set_optional.symbol_whose_bitmap_is_flipped.value(),
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      deserialized_position.symbol_whose_bitmap_is_flipped.value()
   );

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}