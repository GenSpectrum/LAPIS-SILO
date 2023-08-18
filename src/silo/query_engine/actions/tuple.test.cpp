#include "silo/query_engine/actions/tuple.h"
#include "silo/query_engine/actions/action.h"

#include <gtest/gtest.h>

std::pair<silo::storage::ColumnGroup, silo::storage::ColumnPartitionGroup>
createSinglePartitionColumns() {
   std::pair<silo::storage::ColumnGroup, silo::storage::ColumnPartitionGroup> return_value;
   auto& group = return_value.first;
   auto& partitionGroup = return_value.second;

   {
      const std::string string_column_name = "dummy_string_column";
      group.string_columns.emplace(string_column_name, silo::storage::column::StringColumn());
      partitionGroup.metadata.push_back({string_column_name, silo::config::ColumnType::STRING});
      partitionGroup.string_columns.emplace(
         string_column_name, group.string_columns.at(string_column_name).createPartition()
      );
      partitionGroup.string_columns.at(string_column_name).insert("ABCD");
      partitionGroup.string_columns.at(string_column_name)
         .insert(
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string "
         );
      partitionGroup.string_columns.at(string_column_name).insert("ABCD");
   }
   {
      const std::string indexed_string_column_name = "dummy_indexed_string_column";
      group.indexed_string_columns.emplace(
         indexed_string_column_name, silo::storage::column::IndexedStringColumn()
      );
      partitionGroup.metadata.push_back(
         {indexed_string_column_name, silo::config::ColumnType::INDEXED_STRING}
      );
      partitionGroup.indexed_string_columns.emplace(
         indexed_string_column_name,
         group.indexed_string_columns.at(indexed_string_column_name).createPartition()
      );
      partitionGroup.indexed_string_columns.at(indexed_string_column_name).insert("ABCD");
      partitionGroup.indexed_string_columns.at(indexed_string_column_name)
         .insert(
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string "
         );
      partitionGroup.indexed_string_columns.at(indexed_string_column_name).insert("ABCD");
   }
   {
      const std::string int_column_name = "dummy_int_column";
      partitionGroup.metadata.push_back({int_column_name, silo::config::ColumnType::INT});
      group.int_columns.emplace(int_column_name, silo::storage::column::IntColumn());
      partitionGroup.int_columns.emplace(
         int_column_name, group.int_columns.at(int_column_name).createPartition()
      );
      partitionGroup.int_columns.at(int_column_name).insert("42");
      partitionGroup.int_columns.at(int_column_name).insert("-12389172");
      partitionGroup.int_columns.at(int_column_name).insert("42");
   }
   {
      const std::string float_column_name = "dummy_float_column";
      partitionGroup.metadata.push_back({float_column_name, silo::config::ColumnType::FLOAT});
      group.float_columns.emplace(float_column_name, silo::storage::column::FloatColumn());
      partitionGroup.float_columns.emplace(
         float_column_name, group.float_columns.at(float_column_name).createPartition()
      );
      partitionGroup.float_columns.at(float_column_name).insert("42.1");
      partitionGroup.float_columns.at(float_column_name).insert("-12389172.24222");
      partitionGroup.float_columns.at(float_column_name).insert("42.1");
   }
   {
      const std::string date_column_name = "dummy_date_column";
      partitionGroup.metadata.push_back({date_column_name, silo::config::ColumnType::DATE});
      group.date_columns.emplace(date_column_name, silo::storage::column::DateColumn(false));
      partitionGroup.date_columns.emplace(
         date_column_name, group.date_columns.at(date_column_name).createPartition()
      );
      partitionGroup.date_columns.at(date_column_name)
         .insert(silo::common::stringToDate("2023-01-01"));
      partitionGroup.date_columns.at(date_column_name)
         .insert(silo::common::stringToDate("2023-01-01"));
      partitionGroup.date_columns.at(date_column_name)
         .insert(silo::common::stringToDate("2023-01-01"));
   }

   return return_value;
}

using silo::query_engine::actions::Tuple;
using silo::query_engine::actions::TupleFactory;

TEST(Tuple, getsCreatedAndReturnsItsFieldsSuccessfully1) {
   auto columns = createSinglePartitionColumns();
   std::vector<std::byte> data(silo::query_engine::actions::getTupleSize(columns.second.metadata));
   TupleFactory factory(columns.second, columns.second.metadata);
   Tuple under_test = factory.allocateOne(0);

   auto fields = under_test.getFields();
   ASSERT_TRUE(fields.contains("dummy_date_column"));
   ASSERT_TRUE(fields.at("dummy_date_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_date_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_date_column").value()), "2023-01-01");

   ASSERT_TRUE(fields.contains("dummy_float_column"));
   ASSERT_TRUE(fields.at("dummy_float_column").has_value());
   ASSERT_TRUE(holds_alternative<double>(fields.at("dummy_float_column").value()));
   ASSERT_EQ(get<double>(fields.at("dummy_float_column").value()), 42.1);

   ASSERT_TRUE(fields.contains("dummy_int_column"));
   ASSERT_TRUE(fields.at("dummy_int_column").has_value());
   ASSERT_TRUE(holds_alternative<int32_t>(fields.at("dummy_int_column").value()));
   ASSERT_EQ(get<int32_t>(fields.at("dummy_int_column").value()), 42);

   ASSERT_TRUE(fields.contains("dummy_indexed_string_column"));
   ASSERT_TRUE(fields.at("dummy_indexed_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_indexed_string_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_indexed_string_column").value()), "ABCD");

   ASSERT_TRUE(fields.contains("dummy_string_column"));
   ASSERT_TRUE(fields.at("dummy_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_string_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_string_column").value()), "ABCD");
}

TEST(Tuple, getsCreatedAndReturnsItsFieldsSuccessfully2) {
   auto columns = createSinglePartitionColumns();
   std::vector<std::byte> data(silo::query_engine::actions::getTupleSize(columns.second.metadata));
   TupleFactory factory(columns.second, columns.second.metadata);
   Tuple under_test = factory.allocateOne(1);

   auto fields = under_test.getFields();
   ASSERT_TRUE(fields.contains("dummy_date_column"));
   ASSERT_TRUE(fields.at("dummy_date_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_date_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_date_column").value()), "2023-01-01");

   ASSERT_TRUE(fields.contains("dummy_float_column"));
   ASSERT_TRUE(fields.at("dummy_float_column").has_value());
   ASSERT_TRUE(holds_alternative<double>(fields.at("dummy_float_column").value()));
   ASSERT_EQ(get<double>(fields.at("dummy_float_column").value()), -12389172.24222);

   ASSERT_TRUE(fields.contains("dummy_int_column"));
   ASSERT_TRUE(fields.at("dummy_int_column").has_value());
   ASSERT_TRUE(holds_alternative<int32_t>(fields.at("dummy_int_column").value()));
   ASSERT_EQ(get<int32_t>(fields.at("dummy_int_column").value()), -12389172);

   ASSERT_TRUE(fields.contains("dummy_indexed_string_column"));
   ASSERT_TRUE(fields.at("dummy_indexed_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_indexed_string_column").value()));
   ASSERT_EQ(
      get<std::string>(fields.at("dummy_indexed_string_column").value()),
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string "
   );

   ASSERT_TRUE(fields.contains("dummy_string_column"));
   ASSERT_TRUE(fields.at("dummy_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_string_column").value()));
   ASSERT_EQ(
      get<std::string>(fields.at("dummy_string_column").value()),
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string "
   );
}

TEST(TupleFactory, allocatesOneAllocatesManyEqual) {
   auto columns = createSinglePartitionColumns();
   std::vector<std::byte> data(silo::query_engine::actions::getTupleSize(columns.second.metadata));
   TupleFactory factory(columns.second, columns.second.metadata);
   Tuple under_test1 = factory.allocateOne(0);
   auto under_test_vector = factory.allocateMany(1);
   ASSERT_EQ(under_test_vector.size(), 1);
   factory.overwrite(under_test_vector.front(), 0);
   ASSERT_EQ(under_test1, under_test_vector.front());
}

TEST(Tuple, equalityOperatorEquatesCorrectly) {
   auto columns = createSinglePartitionColumns();
   std::vector<std::byte> data(silo::query_engine::actions::getTupleSize(columns.second.metadata));
   TupleFactory factory(columns.second, columns.second.metadata);
   Tuple under_test0a = factory.allocateOne(0);
   Tuple under_test0b = factory.allocateOne(0);
   Tuple under_test1 = factory.allocateOne(1);
   Tuple under_test2 = factory.allocateOne(2);
   ASSERT_EQ(under_test0a, under_test0b);
   ASSERT_NE(under_test0a, under_test1);
   ASSERT_EQ(under_test0a, under_test2);
   ASSERT_NE(under_test0b, under_test1);
   ASSERT_EQ(under_test0b, under_test2);
   ASSERT_NE(under_test1, under_test2);
}

TEST(Tuple, comparesFieldsCorrect) {
   auto columns = createSinglePartitionColumns();
   std::vector<std::byte> data(silo::query_engine::actions::getTupleSize(columns.second.metadata));
   TupleFactory factory(columns.second, columns.second.metadata);
   Tuple tuple0a = factory.allocateOne(0);
   Tuple tuple0b = factory.allocateOne(0);
   Tuple tuple1 = factory.allocateOne(1);
   Tuple tuple2 = factory.allocateOne(2);

   std::vector<silo::query_engine::actions::OrderByField> order_by_fields;
   order_by_fields.push_back({"dummy_indexed_string_column", true});
   Tuple::Comparator under_test = Tuple::getComparator(columns.second.metadata, order_by_fields);

   ASSERT_FALSE(under_test(tuple0a, tuple0b));
   ASSERT_FALSE(under_test(tuple0b, tuple0a));

   ASSERT_TRUE(under_test(tuple0a, tuple1));
   ASSERT_FALSE(under_test(tuple1, tuple0a));

   ASSERT_FALSE(under_test(tuple0a, tuple2));
   ASSERT_FALSE(under_test(tuple2, tuple0a));

   ASSERT_TRUE(under_test(tuple0b, tuple1));
   ASSERT_FALSE(under_test(tuple1, tuple0b));

   ASSERT_FALSE(under_test(tuple0b, tuple2));
   ASSERT_FALSE(under_test(tuple2, tuple0b));

   ASSERT_FALSE(under_test(tuple1, tuple2));
   ASSERT_TRUE(under_test(tuple2, tuple1));

   order_by_fields.clear();
   Tuple::Comparator under_test2 = Tuple::getComparator(columns.second.metadata, order_by_fields);

   ASSERT_FALSE(under_test2(tuple0a, tuple0b));
   ASSERT_FALSE(under_test2(tuple0b, tuple0a));

   ASSERT_FALSE(under_test2(tuple0a, tuple1));
   ASSERT_FALSE(under_test2(tuple1, tuple0a));

   ASSERT_FALSE(under_test2(tuple0a, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple0a));

   ASSERT_FALSE(under_test2(tuple0b, tuple1));
   ASSERT_FALSE(under_test2(tuple1, tuple0b));

   ASSERT_FALSE(under_test2(tuple0b, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple0b));

   ASSERT_FALSE(under_test2(tuple1, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple1));

   order_by_fields.push_back({"dummy_date_column", true});
   Tuple::Comparator under_test3 = Tuple::getComparator(columns.second.metadata, order_by_fields);

   ASSERT_FALSE(under_test3(tuple0a, tuple0b));
   ASSERT_FALSE(under_test3(tuple0b, tuple0a));

   ASSERT_FALSE(under_test3(tuple0a, tuple1));
   ASSERT_FALSE(under_test3(tuple1, tuple0a));

   ASSERT_FALSE(under_test3(tuple0a, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple0a));

   ASSERT_FALSE(under_test3(tuple0b, tuple1));
   ASSERT_FALSE(under_test3(tuple1, tuple0b));

   ASSERT_FALSE(under_test3(tuple0b, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple0b));

   ASSERT_FALSE(under_test3(tuple1, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple1));

   order_by_fields.push_back({"dummy_string_column", false});
   Tuple::Comparator under_test4 = Tuple::getComparator(columns.second.metadata, order_by_fields);

   ASSERT_FALSE(under_test4(tuple0a, tuple0b));
   ASSERT_FALSE(under_test4(tuple0b, tuple0a));

   ASSERT_FALSE(under_test4(tuple0a, tuple1));
   ASSERT_TRUE(under_test4(tuple1, tuple0a));

   ASSERT_FALSE(under_test4(tuple0a, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple0a));

   ASSERT_FALSE(under_test4(tuple0b, tuple1));
   ASSERT_TRUE(under_test4(tuple1, tuple0b));

   ASSERT_FALSE(under_test4(tuple0b, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple0b));

   ASSERT_TRUE(under_test4(tuple1, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple1));
}
