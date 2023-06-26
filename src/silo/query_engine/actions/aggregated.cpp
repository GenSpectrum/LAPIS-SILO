#include "silo/query_engine/actions/aggregated.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <utility>
#include <vector>

#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for_each.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/metadata_store.h"

namespace silo::query_engine::actions {

size_t getTupleSize(std::vector<config::DatabaseMetadata>& group_by_metadata) {
   size_t size = 0;
   for (const auto& metadata : group_by_metadata) {
      if (metadata.getColumnType() == config::ColumnType::STRING) {
         size += common::STRING_SIZE + 4;
      } else {
         size += 4;
      }
   }
   return size;
}

struct Tuple {
   std::vector<char> data;

   Tuple(
      uint32_t sequence_id,
      const silo::MetadataStore& meta_store,
      std::vector<config::DatabaseMetadata>& group_by_metadata,
      size_t tuple_size
   ) {
      data.resize(tuple_size);
      char* data_pointer = data.data();
      for (const auto& metadata : group_by_metadata) {
         if (metadata.getColumnType() == config::ColumnType::DATE) {
            const common::Date value =
               meta_store.date_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<common::Date*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INT) {
            const int32_t value = meta_store.int_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<int32_t*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::STRING) {
            const common::String<common::STRING_SIZE> value =
               meta_store.string_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<common::String<common::STRING_SIZE>*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_PANGOLINEAGE) {
            const silo::Idx value =
               meta_store.pango_lineage_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<silo::Idx*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_STRING) {
            const silo::Idx value =
               meta_store.indexed_string_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<silo::Idx*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else {
            throw std::runtime_error("Unchecked column type of column " + metadata.name);
         }
      }
   }

   bool operator==(const Tuple& other) const { return this->data == other.data; }
};

}  // namespace silo::query_engine::actions

template <>
struct std::hash<silo::query_engine::actions::Tuple> {
   std::size_t operator()(const silo::query_engine::actions::Tuple& tuple) const {
      const std::string_view strView(tuple.data.data(), tuple.data.size());
      return std::hash<std::string_view>{}(strView);
   }
};

namespace silo::query_engine::actions {

struct OrderByField {
   std::string name;
   bool ascending;
};

std::string stringToLowerCase(std::string str) {
   std::transform(str.begin(), str.end(), str.begin(), [](unsigned char character) {
      return std::tolower(character);
   });
   return str;
}

OrderByField parseOrderByField(const std::string& order_by_field) {
   bool ascending = true;

   std::string accessed_column = order_by_field;

   auto space_position = order_by_field.find(' ');
   if (space_position != order_by_field.size()) {
      accessed_column = order_by_field.substr(0, space_position);
      const std::string additional_instruction =
         stringToLowerCase(order_by_field.substr(space_position + 1));
      if (additional_instruction == "asc" || additional_instruction == "ascending") {
         ascending = true;
      } else if (additional_instruction == "desc" || additional_instruction == "descending") {
         ascending = false;
      }
   }

   return OrderByField{accessed_column, ascending};
}

void applyOrderByAndLimit(
   std::vector<AggregationResult>& result,
   const std::vector<OrderByField>& order_by_fields,
   std::optional<uint32_t> limit
) {
   auto cmp = [&order_by_fields](const AggregationResult& value1, const AggregationResult& value2) {
      for (const OrderByField& field : order_by_fields) {
         if (value1.fields.at(field.name) < value2.fields.at(field.name)) {
            return field.ascending;
         }
         if (value2.fields.at(field.name) < value1.fields.at(field.name)) {
            return !field.ascending;
         }
      }
      return false;
   };
   if (limit.has_value() && limit.value() < result.size()) {
      std::partial_sort(result.begin(), result.begin() + limit.value(), result.end(), cmp);
      result.resize(limit.value());
   } else {
      std::sort(result.begin(), result.end(), cmp);
   }
}

std::vector<AggregationResult> generateResult(
   std::unordered_map<Tuple, uint32_t>& tuples,
   const std::vector<config::DatabaseMetadata>& field_defs,
   const Database& database
) {
   std::vector<AggregationResult> result;
   result.reserve(tuples.size());
   for (auto& [key, count] : tuples) {
      std::map<std::string, std::variant<std::string, int32_t>> fields;
      const char* data_pointer = key.data.data();
      for (const auto& metadata : field_defs) {
         if (metadata.getColumnType() == config::ColumnType::DATE) {
            const common::Date value = *reinterpret_cast<const common::Date*>(data_pointer);
            fields[metadata.name] = common::dateToString(value);
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INT) {
            const int32_t value = *reinterpret_cast<const int32_t*>(data_pointer);
            fields[metadata.name] = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::STRING) {
            const common::String<common::STRING_SIZE> value =
               *reinterpret_cast<const common::String<common::STRING_SIZE>*>(data_pointer);
            fields[metadata.name] = database.getStringColumn(metadata.name).lookupValue(value);
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_PANGOLINEAGE) {
            const silo::Idx value = *reinterpret_cast<const silo::Idx*>(data_pointer);
            fields[metadata.name] =
               database.getPangoLineageColumn(metadata.name).lookupValue(value).value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_STRING) {
            const silo::Idx value = *reinterpret_cast<const silo::Idx*>(data_pointer);
            fields[metadata.name] =
               database.getIndexedStringColumn(metadata.name).lookupValue(value);
            data_pointer += sizeof(decltype(value));
         } else {
            throw std::runtime_error("Unchecked column type of column " + metadata.name);
         }
      }
      fields["count"] = static_cast<int32_t>(count);
      result.push_back({fields});
   }
   return result;
}

QueryResult aggregateWithoutGrouping(const std::vector<OperatorResult>& bitmap_filters) {
   uint32_t count = 0;
   for (const auto& filter : bitmap_filters) {
      count += filter->cardinality();
   };
   std::map<std::string, std::variant<std::string, int32_t>> tuple_fields;
   tuple_fields["count"] = static_cast<int32_t>(count);
   return QueryResult{std::vector<AggregationResult>{{tuple_fields}}};
}

Aggregated::Aggregated(
   std::vector<std::string> group_by_fields,
   std::vector<std::string> order_by_fields,
   std::optional<uint32_t> limit
)
    : group_by_fields(std::move(group_by_fields)),
      order_by_fields(std::move(order_by_fields)),
      limit(limit) {}

QueryResult Aggregated::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filters
) const {
   if (group_by_fields.empty()) {
      return aggregateWithoutGrouping(bitmap_filters);
   }
   if (group_by_fields.size() == 1) {
      const std::string group_by_field = group_by_fields[0];
      const auto& metadata = database.database_config.getMetadata(group_by_field);
      if (metadata.name == database.database_config.schema.primary_key) {
         throw QueryParseException("Cannot group by primary key field: '" + group_by_field + "'");
      }
      // TODO(#133) optimize when equal to partition_by field
      // TODO(#133) optimize single field groupby
   }
   std::vector<config::DatabaseMetadata> group_by_metadata;
   for (const std::string& group_by_field : group_by_fields) {
      const auto& metadata = database.database_config.getMetadata(group_by_field);
      group_by_metadata.push_back(metadata);
   }

   std::vector<OrderByField> order_by_definition;
   bool randomize_order = false;
   for (const std::string& order_by_field : order_by_fields) {
      if (order_by_field == "random()") {
         randomize_order = true;
         continue;
      }
      order_by_definition.push_back(parseOrderByField(order_by_field));
   }

   const size_t tuple_size = getTupleSize(group_by_metadata);

   tbb::enumerable_thread_specific<std::unordered_map<Tuple, uint32_t>> maps;

   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, database.partitions.size()),
      [&](tbb::blocked_range<uint32_t> range) {
         std::unordered_map<Tuple, uint32_t>& map = maps.local();
         for (uint32_t partition_id = range.begin(); partition_id != range.end(); ++partition_id) {
            const silo::DatabasePartition& partition = database.partitions[partition_id];
            for (const uint32_t sequence_id : *bitmap_filters[partition_id]) {
               ++map[Tuple(sequence_id, partition.meta_store, group_by_metadata, tuple_size)];
            }
         }
      }
   );
   std::unordered_map<Tuple, uint32_t> final_map;
   for (auto& map : maps) {
      for (auto& [key, value] : map) {
         final_map[key] += value;
      }
   }
   std::vector<AggregationResult> result = generateResult(final_map, group_by_metadata, database);
   if (randomize_order) {
      const uint32_t time_based_seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine rng(time_based_seed);
      std::shuffle(result.begin(), result.end(), rng);
   }
   if (!order_by_definition.empty()) {
      applyOrderByAndLimit(result, order_by_definition, limit);
   }
   return {result};
}

void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action) {
   const std::vector<std::string> group_by_fields =
      json.value("groupByFields", std::vector<std::string>());
   const std::vector<std::string> order_by_fields =
      json.value("orderByFields", std::vector<std::string>());
   const std::optional<uint32_t> limit =
      json.contains("limit") ? std::optional<uint32_t>(json["limit"]) : std::nullopt;
   action = std::make_unique<Aggregated>(group_by_fields, order_by_fields, limit);
}

}  // namespace silo::query_engine::actions
