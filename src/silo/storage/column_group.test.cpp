#include "silo/storage/column_group.h"

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <simdjson.h>
#include <simdutf.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date32_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/zstd/zstd_compressor.h"
#include "silo/zstd/zstd_dictionary.h"

using silo::Nucleotide;
using silo::schema::ColumnIdentifier;
using silo::storage::ColumnGroup;
using silo::storage::column::BoolColumn;
using silo::storage::column::Column;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::Date32Column;
using silo::storage::column::FloatColumn;
using silo::storage::column::IntColumn;
using silo::storage::column::SequenceColumn;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::StringColumn;
using silo::storage::column::StringColumnMetadata;

namespace {

template <Column ColumnType>
std::expected<void, std::string> setupColumnAndInsertJson(
   const std::string& column_name,
   const std::string& json_string
) {
   std::unique_ptr<typename ColumnType::Metadata> meta;
   if constexpr (std::is_same_v<ColumnType, SequenceColumn<Nucleotide>>) {
      meta = std::make_unique<SequenceColumnMetadata<Nucleotide>>(
         column_name, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
      );
   } else {
      meta = std::make_unique<typename ColumnType::Metadata>(column_name);
   }
   ColumnGroup partition_group;
   partition_group.getColumns<ColumnType>().emplace(column_name, ColumnType{meta.get()});

   simdjson::ondemand::parser parser;
   const simdjson::padded_string json(json_string);
   auto doc = parser.iterate(json).value_unsafe();
   simdjson::ondemand::value val = doc[column_name].value_unsafe();

   return partition_group.addJsonValueToColumn(
      ColumnIdentifier{column_name, ColumnType::TYPE}, val
   );
}

std::expected<void, std::string> setupNucleotideColumnAndInsertJson(
   const std::string& column_name,
   const std::vector<Nucleotide::Symbol>& reference,
   const std::string& json_string
) {
   auto meta = std::make_unique<SequenceColumnMetadata<Nucleotide>>(
      column_name, std::vector<Nucleotide::Symbol>{reference}
   );
   ColumnGroup partition_group;
   partition_group.getColumns<SequenceColumn<Nucleotide>>().emplace(
      column_name, SequenceColumn<Nucleotide>{meta.get()}
   );

   simdjson::ondemand::parser parser;
   const simdjson::padded_string json(json_string);
   auto doc = parser.iterate(json).value_unsafe();
   simdjson::ondemand::value val = doc[column_name].value_unsafe();

   return partition_group.addJsonValueToColumn(
      ColumnIdentifier{.name = column_name, .type = SequenceColumn<Nucleotide>::TYPE}, val
   );
}

std::string compressAndBase64Encode(std::string_view sequence, const std::string& reference) {
   auto cdict = std::make_shared<silo::ZstdCDictionary>(reference, 3);
   silo::ZstdCompressor compressor(cdict);
   const auto compressed = compressor.compress(sequence.data(), sequence.size());

   const size_t base64_len = simdutf::base64_length_from_binary(compressed.size());
   std::string encoded(base64_len, '\0');
   simdutf::binary_to_base64(compressed.data(), compressed.size(), encoded.data());
   return encoded;
}

}  // namespace

TEST(ColumnGroup, givenIntegerValueForBoolColumn_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<BoolColumn>("bool_col", R"({"bool_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'bool_col': error getting value as boolean: 42."
      )
   );
}

TEST(ColumnGroup, givenStringValueForIntColumn_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<IntColumn>("int_col", R"({"int_col": "hello"})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'int_col': error getting value as int32: \"hello\"."
      )
   );
}

TEST(ColumnGroup, givenStringValueForFloatColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<FloatColumn>("float_col", R"({"float_col": "hello"})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'float_col': error getting value as double: \"hello\"."
      )
   );
}

TEST(ColumnGroup, givenIntegerValueForStringColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<StringColumn>("string_col", R"({"string_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'string_col': error getting value as string: 42."
      )
   );
}

TEST(ColumnGroup, givenIntegerValueForDate32Column_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<Date32Column>("date_col", R"({"date_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'date_col': error getting value as string: 42."
      )
   );
}

TEST(ColumnGroup, givenObjectMissingSequenceField_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<SequenceColumn<Nucleotide>>("nuc_col", R"({"nuc_col": {}})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'nuc_col': error getting field 'sequence' in object:"
      )
   );
}

TEST(ColumnGroup, givenObjectMissingInsertionsField_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<SequenceColumn<Nucleotide>>(
      "nuc_col", R"({"nuc_col": {"sequence": "A"}})"
   );

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'nuc_col': error getting field 'insertions' in object:"
      )
   );
}

TEST(ColumnGroup, givenValidSequenceCompressed_succeeds) {
   const std::vector<Nucleotide::Symbol> reference = {
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };
   const std::string reference_str = "ACGT";
   const std::string encoded = compressAndBase64Encode("ACGT", reference_str);

   const auto result = setupNucleotideColumnAndInsertJson(
      "nuc_col",
      reference,
      fmt::format(R"({{"nuc_col": {{"sequenceCompressed": "{}", "insertions": []}}}})", encoded)
   );

   ASSERT_TRUE(result.has_value());
}

TEST(ColumnGroup, givenSequenceCompressedWithMutation_succeeds) {
   const std::vector<Nucleotide::Symbol> reference = {
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };
   const std::string reference_str = "ACGT";
   // Sequence differs from reference at position 1 (C → T)
   const std::string encoded = compressAndBase64Encode("ATGT", reference_str);

   const auto result = setupNucleotideColumnAndInsertJson(
      "nuc_col",
      reference,
      fmt::format(R"({{"nuc_col": {{"sequenceCompressed": "{}", "insertions": []}}}})", encoded)
   );

   ASSERT_TRUE(result.has_value());
}

TEST(ColumnGroup, givenSequenceCompressedMultipleRows_succeeds) {
   std::vector<Nucleotide::Symbol> reference = {
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };
   const std::string reference_str = "ACGT";

   auto meta =
      std::make_unique<SequenceColumnMetadata<Nucleotide>>("nuc_col", std::move(reference));
   ColumnGroup partition_group;
   partition_group.getColumns<SequenceColumn<Nucleotide>>().emplace(
      "nuc_col", SequenceColumn<Nucleotide>{meta.get()}
   );

   for (const std::string_view sequence : {"ACGT", "ATGT", "ACGT"}) {
      const std::string encoded = compressAndBase64Encode(sequence, reference_str);
      const std::string json =
         fmt::format(R"({{"nuc_col": {{"sequenceCompressed": "{}", "insertions": []}}}})", encoded);

      simdjson::ondemand::parser parser;
      const simdjson::padded_string padded(json);
      auto doc = parser.iterate(padded).value_unsafe();
      simdjson::ondemand::value val = doc["nuc_col"].value_unsafe();

      const auto result = partition_group.addJsonValueToColumn(
         ColumnIdentifier{.name = "nuc_col", .type = SequenceColumn<Nucleotide>::TYPE}, val
      );
      ASSERT_TRUE(result.has_value());
   }
}

TEST(ColumnGroup, givenSequenceCompressedWithInvalidBase64_returnsError) {
   const std::vector<Nucleotide::Symbol> reference = {Nucleotide::Symbol::A};

   const auto result = setupNucleotideColumnAndInsertJson(
      "nuc_col",
      reference,
      R"({"nuc_col": {"sequenceCompressed": "not!valid@base64#", "insertions": []}})"
   );

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(result.error(), testing::HasSubstr("invalid base64"));
}

TEST(ColumnGroup, givenSequenceCompressedWithInvalidZstdData_returnsError) {
   const std::vector<Nucleotide::Symbol> reference = {
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };
   // Valid base64 but the decoded bytes are not valid zstd-compressed data
   const std::string random_bytes = "\x01\x02\x03\x04\x05\x06\x07\x08";
   const size_t base64_len = simdutf::base64_length_from_binary(random_bytes.size());
   std::string encoded(base64_len, '\0');
   simdutf::binary_to_base64(random_bytes.data(), random_bytes.size(), encoded.data());

   const auto result = setupNucleotideColumnAndInsertJson(
      "nuc_col",
      reference,
      fmt::format(R"({{"nuc_col": {{"sequenceCompressed": "{}", "insertions": []}}}})", encoded)
   );

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(result.error(), testing::HasSubstr("failed to decompress"));
}
