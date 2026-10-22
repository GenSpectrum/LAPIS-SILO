#include "rhydb/schema/data_type.h"

#include <optional>
#include <sstream>

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using rhydb::schema::DataType;
using Kind = rhydb::schema::DataType::Kind;
using rhydb::schema::dataTypeToString;

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
   const DataType dictionary_string = DataType::dictionaryEncoded(DataType::STRING);
   EXPECT_EQ(dictionary_string.getKind(), Kind::DICTIONARY_ENCODED);
   EXPECT_EQ(dictionary_string.getDictionaryValueType(), DataType::STRING);
   EXPECT_EQ(dictionary_string.getPlainType(), DataType::STRING);
   EXPECT_EQ(DataType::INT32.getPlainType(), DataType::INT32);

   EXPECT_EQ(dictionary_string, DataType::dictionaryEncoded(DataType::STRING));
   EXPECT_NE(dictionary_string, DataType::STRING);
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

TEST(DataType, hasDisplayNames) {
   EXPECT_EQ(dataTypeToString(DataType::ZSTD_COMPRESSED_STRING), "ZSTD_COMPRESSED_STRING");
   EXPECT_EQ(
      dataTypeToString(DataType::dictionaryEncoded(DataType::STRING)), "DICTIONARY_ENCODED(STRING)"
   );
}

TEST(DataType, hasSchemaTypeNames) {
   using rhydb::schema::fromSchemaTypeName;
   using rhydb::schema::toSchemaTypeName;
   for (const DataType& type : {
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
