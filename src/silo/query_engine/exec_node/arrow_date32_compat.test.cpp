#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <arrow/array/builder_binary.h>
#include <gtest/gtest.h>

#include <arrow/array/array_primitive.h>
#include <arrow/array/builder_primitive.h>
#include <arrow/compute/api.h>
#include <arrow/result.h>
#include <arrow/scalar.h>
#include <arrow/status.h>
#include <arrow/type.h>
#include <arrow/type_fwd.h>

#include "silo/common/date32.h"

namespace {

using silo::common::Date32;
using silo::common::date32ToString;
using silo::common::stringToDate32;

TEST(SiloDate32MatchesArrowDate32, cTypesAreIdentical) {
   // SILO's Date32 must be bit-identical to Arrow's Date32 c_type so that one
   // can be reinterpreted as the other without conversion.
   static_assert(std::is_same_v<Date32, int32_t>);
   static_assert(std::is_same_v<arrow::Date32Type::c_type, int32_t>);
   EXPECT_EQ(arrow::date32()->id(), arrow::Type::DATE32);
}

TEST(SiloDate32MatchesArrowDate32, dateUnitIsDays) {
   // SILO encodes Date32 as days since 1970-01-01. Arrow's Date32 uses the
   // same encoding (DateUnit::DAY since UNIX epoch).
   EXPECT_EQ(arrow::Date32Type::UNIT, arrow::DateUnit::DAY);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(SiloDate32MatchesArrowDate32, builderAcceptsSiloDate32Values) {
   const std::vector<std::string> dates = {
      "1970-01-01",  // epoch
      "1969-12-31",  // pre-epoch
      "2000-02-29",  // century leap year
      "2024-02-29",  // standard leap year
      "1900-02-28",  // non-leap century
      "2100-02-28",  // non-leap century in the future
      "2020-12-24",
      "2099-12-31",
      "1900-01-01",
   };

   arrow::Date32Builder builder;
   for (const auto& date_string : dates) {
      const auto silo_value = stringToDate32(date_string);
      ASSERT_TRUE(silo_value.has_value());
      ASSERT_TRUE(builder.Append(silo_value.value()).ok());
   }

   ASSERT_TRUE(builder.AppendNull().ok());

   std::shared_ptr<arrow::Array> array;
   ASSERT_TRUE(builder.Finish(&array).ok());
   ASSERT_EQ(array->type()->id(), arrow::Type::DATE32);

   const auto& date_array = static_cast<const arrow::Date32Array&>(*array);
   ASSERT_EQ(date_array.length(), static_cast<int64_t>(dates.size() + 1));

   for (size_t i = 0; i < dates.size(); ++i) {
      ASSERT_FALSE(date_array.IsNull(static_cast<int64_t>(i)));
      const int32_t silo_value = stringToDate32(dates.at(i)).value();
      const int32_t arrow_value = date_array.Value(static_cast<int64_t>(i));
      EXPECT_EQ(silo_value, arrow_value) << "Mismatch for date " << dates.at(i);
      EXPECT_EQ(date32ToString(arrow_value), dates.at(i));
   }
   EXPECT_TRUE(date_array.IsNull(static_cast<int64_t>(dates.size())));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(SiloDate32MatchesArrowDate32, arrowCastFromStringMatchesSiloEncoding) {
   // Arrow's "cast from utf8 to date32" must produce the exact same int32
   // value that SILO produces. If this ever diverges, the storage layer and
   // the query result will disagree on what a date is.
   const std::vector<std::string> dates = {
      "1970-01-01",
      "1969-12-31",
      "2000-02-29",
      "2024-02-29",
      "2020-12-24",
      "2099-12-31",
      "1900-01-01",
      "2025-01-01",
   };

   arrow::StringBuilder string_builder;
   for (const auto& date_string : dates) {
      ASSERT_TRUE(string_builder.Append(date_string).ok());
   }
   std::shared_ptr<arrow::Array> string_array;
   ASSERT_TRUE(string_builder.Finish(&string_array).ok());

   arrow::compute::CastOptions cast_options;
   cast_options.to_type = arrow::date32();
   const auto cast_result = arrow::compute::Cast(string_array, cast_options);
   ASSERT_TRUE(cast_result.ok()) << cast_result.status().ToString();

   const auto arrow_dates =
      std::static_pointer_cast<arrow::Date32Array>(cast_result.ValueOrDie().make_array());
   ASSERT_EQ(arrow_dates->length(), static_cast<int64_t>(dates.size()));

   for (size_t i = 0; i < dates.size(); ++i) {
      const int32_t silo_value = stringToDate32(dates.at(i)).value();
      const int32_t arrow_value = arrow_dates->Value(static_cast<int64_t>(i));
      EXPECT_EQ(silo_value, arrow_value) << "Encoding mismatch for date " << dates.at(i);
   }
}

TEST(SiloDate32MatchesArrowDate32, scalarToStringMatchesSiloFormatting) {
   // Round-trip through arrow::Date32Scalar to ensure the same int32 value
   // is interpreted as the same calendar day.
   struct Sample {
      std::string string_value;
      int32_t expected_int;
   };
   const std::vector<Sample> samples = {
      {.string_value = "1970-01-01", .expected_int = 0},
      {.string_value = "1969-12-31", .expected_int = -1},
      {.string_value = "1969-12-30", .expected_int = -2},
      {.string_value = "2020-01-01", .expected_int = 18262},
      {.string_value = "2010-12-03", .expected_int = 14946},
   };
   for (const auto& sample : samples) {
      const auto silo_value = stringToDate32(sample.string_value);
      ASSERT_TRUE(silo_value.has_value());
      EXPECT_EQ(silo_value.value(), sample.expected_int);

      const arrow::Date32Scalar scalar{sample.expected_int};
      EXPECT_EQ(scalar.value, sample.expected_int);
      EXPECT_EQ(date32ToString(scalar.value), sample.string_value);
   }
}

TEST(SiloDate32MatchesArrowDate32, dayArithmeticAgreesWithArrow) {
   const auto jan1 = stringToDate32("2023-01-01").value();
   const auto jan2 = stringToDate32("2023-01-02").value();
   const auto dec31 = stringToDate32("2023-12-31").value();

   arrow::Date32Builder builder;
   ASSERT_TRUE(builder.Append(jan1).ok());
   ASSERT_TRUE(builder.Append(jan2).ok());
   ASSERT_TRUE(builder.Append(dec31).ok());

   std::shared_ptr<arrow::Array> array;
   ASSERT_TRUE(builder.Finish(&array).ok());
   const auto& date_array = static_cast<const arrow::Date32Array&>(*array);

   EXPECT_EQ(date_array.Value(1) - date_array.Value(0), 1);
   EXPECT_EQ(date_array.Value(2) - date_array.Value(0), 364);
}

TEST(SiloDate32MatchesArrowDate32, extremalInt32ValuesRoundTripBitwise) {
   // The Arrow Date32Builder accepts the full int32_t range. SILO must be
   // able to store and read back any value Arrow produces, even if the
   // resulting calendar date is outside the practical YYYY-MM-DD range.
   const std::vector<int32_t> values = {
      0,
      -1,
      1,
      std::numeric_limits<int32_t>::min(),
      std::numeric_limits<int32_t>::max(),
   };

   arrow::Date32Builder builder;
   for (const auto value : values) {
      ASSERT_TRUE(builder.Append(value).ok());
   }
   std::shared_ptr<arrow::Array> array;
   ASSERT_TRUE(builder.Finish(&array).ok());
   const auto& date_array = static_cast<const arrow::Date32Array&>(*array);

   for (size_t i = 0; i < values.size(); ++i) {
      EXPECT_EQ(date_array.Value(static_cast<int64_t>(i)), values.at(i));
      const Date32 silo_value = date_array.Value(static_cast<int64_t>(i));
      EXPECT_EQ(silo_value, values.at(i));
   }
}

}  // namespace
