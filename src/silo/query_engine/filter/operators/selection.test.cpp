#include "silo/query_engine/filter/operators/selection.h"

#include <gtest/gtest.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/storage/column/int_column.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::filter::operators::BitmapProducer;
using silo::query_engine::filter::operators::Comparator;
using silo::query_engine::filter::operators::CompareToValueSelection;
using silo::query_engine::filter::operators::Operator;
using silo::query_engine::filter::operators::Predicate;
using silo::query_engine::filter::operators::Selection;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::IntColumn;
using silo::storage::column::RowLayout;

namespace {

std::pair<std::shared_ptr<ColumnMetadata>, IntColumn> makeTestColumn(
   const std::vector<int32_t>& values
) {
   auto metadata = std::make_shared<ColumnMetadata>("test");
   IntColumn test_column{metadata.get()};
   IntColumn::Builder builder;
   for (auto value : values) {
      builder.insert(value);
   }
   SILO_ASSERT(test_column.appendChunk(builder.finalize()).has_value());
   return {metadata, test_column};
}

}  // namespace

TEST(OperatorSelection, equalsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::EQUALS, 1),
      row_layout
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 2, 3, 4}));
}

TEST(OperatorSelection, notEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::NOT_EQUALS, 1),
      row_layout
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 2, 3, 4}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, lessShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::LESS, 1),
      row_layout
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(
      negated->evaluate().getConstReference(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9})
   );
}

TEST(OperatorSelection, lessOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(
         test_column, Comparator::LESS_OR_EQUALS, 1
      ),
      row_layout
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({2, 3, 4}));
}

TEST(OperatorSelection, higherOrEqualsShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(
         test_column, Comparator::HIGHER_OR_EQUALS, 1
      ),
      row_layout
   );

   ASSERT_EQ(
      under_test->evaluate().getConstReference(), roaring::Roaring({1, 2, 3, 4, 5, 6, 7, 8, 9})
   );
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0}));
}

TEST(OperatorSelection, higherShouldReturnCorrectValues) {
   const std::vector<int32_t> values({{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   auto under_test = std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::HIGHER, 1),
      row_layout
   );

   ASSERT_EQ(under_test->evaluate().getConstReference(), roaring::Roaring({2, 3, 4}));
   auto negated = Selection::negate(std::move(under_test));
   ASSERT_EQ(negated->evaluate().getConstReference(), roaring::Roaring({0, 1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, correctWithNegativeNumbers) {
   const std::vector<int32_t> values({{0, -1, 4, 4, 4, -1, -1, -1, -1, -1}});
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   const Selection under_test(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::EQUALS, -1),
      row_layout
   );

   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({1, 5, 6, 7, 8, 9}));
}

TEST(OperatorSelection, returnsCorrectTypeInfo) {
   const std::vector<int32_t> values{{0, 1, 4, 4, 4, 1, 1, 1, 1, 1}};
   auto [metadata, test_column] = makeTestColumn(values);
   const auto row_layout = RowLayout::of(values.size());

   const Selection under_test(
      std::make_unique<CompareToValueSelection<IntColumn>>(test_column, Comparator::EQUALS, -1),
      row_layout
   );

   ASSERT_EQ(under_test.type(), silo::query_engine::filter::operators::SELECTION);
}

namespace {

// A column of 100 rows holding value == row index, so global row ids map directly to bitmap bits.
std::pair<std::shared_ptr<ColumnMetadata>, IntColumn> makeRangeTestColumn() {
   std::vector<int32_t> values;
   values.reserve(100);
   for (int32_t i = 0; i < 100; ++i) {
      values.push_back(i);
   }
   return makeTestColumn(values);
}

// value >= 10 AND value < 50, as two separate predicates, matching rows [10, 50).
std::vector<std::unique_ptr<Predicate>> makeRangePredicates(const IntColumn& column) {
   std::vector<std::unique_ptr<Predicate>> predicates;
   predicates.push_back(
      std::make_unique<CompareToValueSelection<IntColumn>>(column, Comparator::HIGHER_OR_EQUALS, 10)
   );
   predicates.push_back(
      std::make_unique<CompareToValueSelection<IntColumn>>(column, Comparator::LESS, 50)
   );
   return predicates;
}

roaring::Roaring rangeBitmap(uint32_t start, uint32_t end) {
   roaring::Roaring result;
   result.addRange(start, end);
   return result;
}

}  // namespace

TEST(OperatorSelection, multiplePredicatesWithoutChildReturnCorrectValues) {
   auto [metadata, test_column] = makeRangeTestColumn();
   const auto row_layout = RowLayout::of(100);

   const Selection under_test(makeRangePredicates(test_column), row_layout);

   ASSERT_EQ(under_test.evaluate().getConstReference(), rangeBitmap(10, 50));
}

TEST(OperatorSelection, multiplePredicatesWithLargeChildReturnCorrectValues) {
   auto [metadata, test_column] = makeRangeTestColumn();
   const auto row_layout = RowLayout::of(100);

   // Child cardinality (75) exceeds numRows / 10 == 10, so the large-child branch primes candidates
   // from the most-selective predicate.
   const roaring::Roaring child_bitmap = rangeBitmap(5, 80);
   std::unique_ptr<Operator> child = std::make_unique<BitmapProducer>(
      [&]() { return CopyOnWriteBitmap(&child_bitmap); }, row_layout
   );

   const Selection under_test(std::move(child), makeRangePredicates(test_column), row_layout);

   // child [5, 80) intersected with [10, 50) == [10, 50).
   ASSERT_EQ(under_test.evaluate().getConstReference(), rangeBitmap(10, 50));
}

TEST(OperatorSelection, multiplePredicatesWithSmallChildReturnCorrectValues) {
   auto [metadata, test_column] = makeRangeTestColumn();
   const auto row_layout = RowLayout::of(100);

   // Child cardinality (5) is <= numRows / 10 == 10, so the small-child branch matches every child
   // row against all predicates. Rows 5 and 60 and 99 fail the [10, 50) range.
   const roaring::Roaring child_bitmap({5, 15, 45, 60, 99});
   std::unique_ptr<Operator> child = std::make_unique<BitmapProducer>(
      [&]() { return CopyOnWriteBitmap(&child_bitmap); }, row_layout
   );

   const Selection under_test(std::move(child), makeRangePredicates(test_column), row_layout);

   ASSERT_EQ(under_test.evaluate().getConstReference(), roaring::Roaring({15, 45}));
}
