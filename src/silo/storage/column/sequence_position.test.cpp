#include "silo/storage/column/sequence_column.h"

#include <fstream>
#include <iostream>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/array.hpp>

#include "silo/common/optional_bool.h"
#include "silo/database.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/serialize_optional.h"

using silo::Nucleotide;
using silo::storage::column::SequencePosition;

namespace {
void serializeToFile(const std::string& filename, const SequencePosition<Nucleotide>& position) {
   std::ofstream output_file(filename.c_str(), std::ios::binary);
   ::boost::archive::binary_oarchive output_archive(output_file);
   output_archive << position;
   output_file.close();
}

void deserializeFromFile(const std::string& filename, SequencePosition<Nucleotide>& position) {
   std::ifstream input_file(filename, std::ios::binary);
   ::boost::archive::binary_iarchive input_archive(input_file);
   input_archive >> position;
   input_file.close();
}
}  // namespace

TEST(SequencePosition, flipsMostNumerousCorrectlyFromInitiallyUnoptimized) {
   SequencePosition<Nucleotide> under_test;

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), std::nullopt);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, flipsMostNumerousCorrectlyFromInitiallyDifferentSymbolFlipped) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyFlipped(Nucleotide::Symbol::A);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), std::nullopt);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, flipsMostNumerousCorrectlyFromInitiallySameSymbolFlipped) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyFlipped(Nucleotide::Symbol::C);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), std::nullopt);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), std::nullopt);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, deletesMostNumerousCorrectlyFromInitiallyUnoptimized) {
   SequencePosition<Nucleotide> under_test;

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(under_test.deleteMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring());
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));

   ASSERT_THROW(under_test.deleteMostNumerousBitmap(5), std::runtime_error);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring());
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, deletesMostNumerousCorrectlyFromInitiallyDifferentSymbolFlipped) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyDeleted(Nucleotide::Symbol::A);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({1, 2, 3}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring());

   ASSERT_THROW(under_test.deleteMostNumerousBitmap(5), std::runtime_error);
}

TEST(SequencePosition, deletesMostNumerousCorrectlyFromInitiallySameFlippedSymbol) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyDeleted(Nucleotide::Symbol::C);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_THROW(under_test.deleteMostNumerousBitmap(5), std::runtime_error);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring());
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, deletesCorrectlyFromInitiallyDifferentSymbolFlipped) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyFlipped(Nucleotide::Symbol::A);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({1, 2, 3}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({1, 2, 3}));

   ASSERT_EQ(under_test.deleteMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring());
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

TEST(SequencePosition, flipThenDeleteFromInitiallyDifferentSymbolFlipped) {
   SequencePosition<Nucleotide> under_test =
      SequencePosition<Nucleotide>::fromInitiallyFlipped(Nucleotide::Symbol::A);

   under_test.addValues(Nucleotide::Symbol::C, std::vector<uint32_t>{1, 2, 3}, 0, 5);
   under_test.addValues(Nucleotide::Symbol::A, std::vector<uint32_t>{0, 4}, 0, 5);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({1, 2, 3}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({1, 2, 3}));

   ASSERT_EQ(under_test.flipMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring({0, 4}));
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));

   ASSERT_EQ(under_test.deleteMostNumerousBitmap(5), Nucleotide::Symbol::C);

   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::C), roaring::Roaring());
   ASSERT_EQ(*under_test.getBitmap(Nucleotide::Symbol::A), roaring::Roaring({0, 4}));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(SequencePosition, shouldSerializeAndDeserializePositionsWithEmptyOptional) {
   const std::string test_file = "test.bin";

   const SequencePosition<Nucleotide> position_with_unset_optional;
   serializeToFile(test_file, position_with_unset_optional);

   SequencePosition<Nucleotide> deserialized_position;
   deserializeFromFile(test_file, deserialized_position);

   for (const auto& symbol : Nucleotide::SYMBOLS) {
      EXPECT_FALSE(position_with_unset_optional.isSymbolFlipped(symbol));
      EXPECT_FALSE(deserialized_position.isSymbolFlipped(symbol));
   }

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}

TEST(SequencePosition, shouldSerializeAndDeserializePositionWithFlippedBitmap) {
   const std::string test_file = "test.bin";

   const SequencePosition<Nucleotide> position_with_set_optional =
      SequencePosition<Nucleotide>::fromInitiallyFlipped(Nucleotide::Symbol::A);
   serializeToFile(test_file, position_with_set_optional);

   SequencePosition<Nucleotide> deserialized_position;
   deserializeFromFile(test_file, deserialized_position);

   EXPECT_TRUE(deserialized_position.isSymbolFlipped(Nucleotide::Symbol::A));
   EXPECT_TRUE(position_with_set_optional.isSymbolFlipped(Nucleotide::Symbol::A));

   for (const auto& symbol : Nucleotide::SYMBOLS) {
      EXPECT_FALSE(position_with_set_optional.isSymbolDeleted(symbol));
      EXPECT_FALSE(deserialized_position.isSymbolDeleted(symbol));
      if (symbol != Nucleotide::Symbol::A) {
         EXPECT_FALSE(position_with_set_optional.isSymbolFlipped(symbol));
         EXPECT_FALSE(deserialized_position.isSymbolFlipped(symbol));
      }
   }

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}

TEST(SequencePosition, shouldSerializeAndDeserializePositionWithDeletedBitmap) {
   const std::string test_file = "test.bin";

   const SequencePosition<Nucleotide> position_with_set_optional =
      SequencePosition<Nucleotide>::fromInitiallyDeleted(Nucleotide::Symbol::A);
   serializeToFile(test_file, position_with_set_optional);

   SequencePosition<Nucleotide> deserialized_position;
   deserializeFromFile(test_file, deserialized_position);

   EXPECT_TRUE(deserialized_position.isSymbolDeleted(Nucleotide::Symbol::A));
   EXPECT_TRUE(position_with_set_optional.isSymbolDeleted(Nucleotide::Symbol::A));

   for (const auto& symbol : Nucleotide::SYMBOLS) {
      EXPECT_FALSE(position_with_set_optional.isSymbolFlipped(symbol));
      EXPECT_FALSE(deserialized_position.isSymbolFlipped(symbol));
      if (symbol != Nucleotide::Symbol::A) {
         EXPECT_FALSE(position_with_set_optional.isSymbolDeleted(symbol));
         EXPECT_FALSE(deserialized_position.isSymbolDeleted(symbol));
      }
   }

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}