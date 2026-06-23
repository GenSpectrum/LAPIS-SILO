#include "silo/query_engine/expressions/zstd_decompress_scalar.h"

#include <exception>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/query_engine/expressions/expression.h"
#include "silo/schema/database_schema.h"

using silo::query_engine::expressions::Expression;
using silo::query_engine::expressions::ZstdDecompressScalar;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;

namespace {

ColumnIdentifier nucColumn() {
   return ColumnIdentifier{.name = "nuc", .type = ColumnType::NUCLEOTIDE_SEQUENCE};
}

ZstdDecompressScalar makeScalar() {
   return ZstdDecompressScalar{nucColumn(), "ACGT"};
}

TEST(ZstdDecompressScalar, typeIsString) {
   EXPECT_EQ(makeScalar().type(), ColumnType::STRING);
}

TEST(ZstdDecompressScalar, kindMatchesKindConstant) {
   EXPECT_EQ(makeScalar().kind(), ZstdDecompressScalar::KIND);
   EXPECT_EQ(makeScalar().kind(), Expression::Kind::ZSTD_DECOMPRESS_SCALAR);
}

TEST(ZstdDecompressScalar, toStringMentionsInputColumn) {
   EXPECT_EQ(makeScalar().toString(), "zstd_decompress(nuc)");
}

TEST(ZstdDecompressScalar, freeIUsReturnsInputColumn) {
   EXPECT_THAT(makeScalar().freeIUs(), ::testing::ElementsAre(nucColumn()));
}

TEST(ZstdDecompressScalar, cloneProducesEqualExpression) {
   auto original = makeScalar();
   auto cloned = original.clone();

   ASSERT_NE(cloned, nullptr);
   const auto* cloned_scalar = dynamic_cast<const ZstdDecompressScalar*>(cloned.get());
   ASSERT_NE(cloned_scalar, nullptr);
   EXPECT_EQ(cloned_scalar->input_column, original.input_column);
   EXPECT_EQ(cloned_scalar->dictionary_string, original.dictionary_string);
}

TEST(ZstdDecompressScalar, emptyDictionaryThrows) {
   EXPECT_THROW({ ZstdDecompressScalar(nucColumn(), ""); }, std::exception);
}

}  // namespace
