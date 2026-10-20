#include "rhydb/schema/data_type.h"

#include <optional>
#include <sstream>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using rhydb::schema::DataType;
using Kind = rhydb::schema::DataType::Kind;

namespace {

DataType roundTrip(const DataType& type) {
   std::stringstream buffer;
   {
      boost::archive::binary_oarchive output{buffer};
      output << type;
   }
   boost::archive::binary_iarchive input{buffer};
   DataType result;
   input >> result;
   return result;
}

}  // namespace

TEST(DataType, dictionaryEncodedTypeHasItsValueTypeAsChild) {
   constexpr DataType DICTIONARY_STRING = DataType::dictionaryEncoded(DataType::STRING);
   static_assert(DICTIONARY_STRING.getKind() == Kind::DICTIONARY_ENCODED);
   static_assert(DICTIONARY_STRING.getDictionaryValueType() == DataType::STRING);
   static_assert(DICTIONARY_STRING.getPlainType() == DataType::STRING);
   static_assert(DataType::INT32.getPlainType() == DataType::INT32);

   EXPECT_EQ(DICTIONARY_STRING, DataType::dictionaryEncoded(DataType::STRING));
   EXPECT_NE(DICTIONARY_STRING, DataType::STRING);
}

TEST(DataType, isOrderedByKindAndThenByChild) {
   EXPECT_LT(DataType::STRING, DataType::dictionaryEncoded(DataType::STRING));
   EXPECT_LT(DataType::dictionaryEncoded(DataType::STRING), DataType::DATE32);
}

TEST(DataType, canOnlyDictionaryEncodeStrings) {
   EXPECT_TRUE(DataType::canBeDictionaryEncoded(DataType::STRING));
   EXPECT_FALSE(DataType::canBeDictionaryEncoded(DataType::INT32));
   EXPECT_FALSE(DataType::canBeDictionaryEncoded(DataType::dictionaryEncoded(DataType::STRING)));
}

TEST(DataType, survivesSerialization) {
   EXPECT_EQ(roundTrip(DataType::DATE32), DataType::DATE32);
   EXPECT_EQ(
      roundTrip(DataType::dictionaryEncoded(DataType::STRING)),
      DataType::dictionaryEncoded(DataType::STRING)
   );
}

TEST(DataType, hasSchemaTypeNames) {
   using rhydb::schema::fromSchemaTypeName;
   using rhydb::schema::toSchemaTypeName;
   for (const DataType type : {
           DataType::STRING,
           DataType::DATE32,
           DataType::BOOL,
           DataType::INT32,
           DataType::INT64,
           DataType::FLOAT,
           DataType::NUCLEOTIDE_SEQUENCE,
           DataType::AMINO_ACID_SEQUENCE,
           DataType::ZSTD_COMPRESSED_STRING,
        }) {
      EXPECT_EQ(fromSchemaTypeName(toSchemaTypeName(type)), type);
   }
   EXPECT_EQ(toSchemaTypeName(DataType::INT32), "int32");
   EXPECT_EQ(fromSchemaTypeName("int"), DataType::INT32);
   EXPECT_EQ(fromSchemaTypeName("unknown"), std::nullopt);
}
