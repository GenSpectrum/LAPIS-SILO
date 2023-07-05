#include "silo/query_engine/actions/aggregated.h"

#include <chrono>
#include <cmath>
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
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::actions {

using json_value_type = std::optional<std::variant<std::string, int32_t, double>>;

size_t getTupleSize(std::vector<config::DatabaseMetadata>& group_by_metadata) {
   size_t size = 0;
   for (const auto& metadata : group_by_metadata) {
      if (metadata.getColumnType() == config::ColumnType::STRING) {
         size += sizeof(common::String<common::STRING_SIZE>);
      } else if (metadata.getColumnType() == config::ColumnType::FLOAT) {
         size += sizeof(double);
      } else if (metadata.getColumnType() == config::ColumnType::INT) {
         size += sizeof(int32_t);
      } else if (metadata.getColumnType() == config::ColumnType::DATE) {
         size += sizeof(common::Date);
      } else {
         size += sizeof(silo::Idx);
      }
   }
   return size;
}

struct Tuple {
   std::vector<char> data;
   const silo::storage::ColumnGroup& columns;

   Tuple(uint32_t sequence_id, const silo::storage::ColumnGroup& columns, size_t tuple_size)
       : columns(columns) {
      data.resize(tuple_size);
      char* data_pointer = data.data();
      for (const auto& metadata : columns.metadata) {
         if (metadata.getColumnType() == config::ColumnType::DATE) {
            const common::Date value =
               columns.date_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<common::Date*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INT) {
            const int32_t value = columns.int_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<int32_t*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::FLOAT) {
            const double value = columns.float_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<double*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::STRING) {
            const common::String<common::STRING_SIZE> value =
               columns.string_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<common::String<common::STRING_SIZE>*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_PANGOLINEAGE) {
            const silo::Idx value =
               columns.pango_lineage_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<silo::Idx*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_STRING) {
            const silo::Idx value =
               columns.indexed_string_columns.at(metadata.name).getValues()[sequence_id];
            *reinterpret_cast<silo::Idx*>(data_pointer) = value;
            data_pointer += sizeof(decltype(value));
         } else {
            throw std::runtime_error("Unchecked column type of column " + metadata.name);
         }
      }
   }

   [[nodiscard]] std::map<std::string, json_value_type> getFields() const {
      std::map<std::string, json_value_type> fields;
      const char* data_pointer = data.data();
      for (const auto& metadata : columns.metadata) {
         if (metadata.getColumnType() == config::ColumnType::DATE) {
            const common::Date value = *reinterpret_cast<const common::Date*>(data_pointer);
            fields[metadata.name] = common::dateToString(value);
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INT) {
            const int32_t value = *reinterpret_cast<const int32_t*>(data_pointer);
            if (value == INT32_MIN) {
               fields[metadata.name] = std::nullopt;
            } else {
               fields[metadata.name] = value;
            }
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::FLOAT) {
            const double value = *reinterpret_cast<const double*>(data_pointer);
            if (std::isnan(value)) {
               fields[metadata.name] = std::nullopt;
            } else {
               fields[metadata.name] = value;
            }
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::STRING) {
            const common::String<common::STRING_SIZE> value =
               *reinterpret_cast<const common::String<common::STRING_SIZE>*>(data_pointer);
            std::string string_value = columns.string_columns.at(metadata.name).lookupValue(value);
            if (string_value.empty()) {
               fields[metadata.name] = std::nullopt;
            } else {
               fields[metadata.name] = string_value;
            }
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_PANGOLINEAGE) {
            const silo::Idx value = *reinterpret_cast<const silo::Idx*>(data_pointer);
            std::string string_value =
               columns.pango_lineage_columns.at(metadata.name).lookupValue(value).value;
            if (string_value.empty()) {
               fields[metadata.name] = std::nullopt;
            } else {
               fields[metadata.name] = string_value;
            }
            data_pointer += sizeof(decltype(value));
         } else if (metadata.getColumnType() == config::ColumnType::INDEXED_STRING) {
            const silo::Idx value = *reinterpret_cast<const silo::Idx*>(data_pointer);
            std::string string_value =
               columns.indexed_string_columns.at(metadata.name).lookupValue(value);
            if (string_value.empty()) {
               fields[metadata.name] = std::nullopt;
            } else {
               fields[metadata.name] = string_value;
            }
            data_pointer += sizeof(decltype(value));
         } else {
            throw std::runtime_error("Unchecked column type of column " + metadata.name);
         }
      }
      return fields;
   }

   bool operator==(const Tuple& other) const { return this->data == other.data; }
};

}  // namespace silo::query_engine::actions

template <>
struct std::hash<silo::query_engine::actions::Tuple> {
   std::size_t operator()(const silo::query_engine::actions::Tuple& tuple) const {
      const std::string_view str_view(tuple.data.data(), tuple.data.size());
      return std::hash<std::string_view>{}(str_view);
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
   std::vector<QueryResultEntry>& result,
   const std::vector<OrderByField>& order_by_fields,
   std::optional<uint32_t> limit
) {
   auto cmp = [&order_by_fields](const QueryResultEntry& entry1, const QueryResultEntry& entry2) {
      for (const OrderByField& field : order_by_fields) {
         if (entry1.fields.at(field.name) == entry2.fields.at(field.name)) {
            continue;
         }
         return entry1.fields.at(field.name) < entry2.fields.at(field.name) ? field.ascending
                                                                            : !field.ascending;
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
const std::string COUNT_FIELD = "count";

std::vector<QueryResultEntry> generateResult(std::unordered_map<Tuple, uint32_t>& tuple_counts) {
   std::vector<QueryResultEntry> result;
   result.reserve(tuple_counts.size());
   for (auto& [tuple, count] : tuple_counts) {
      std::map<std::string, json_value_type> fields = tuple.getFields();
      fields[COUNT_FIELD] = static_cast<int32_t>(count);
      result.push_back({fields});
   }
   return result;
}

QueryResult aggregateWithoutGrouping(const std::vector<OperatorResult>& bitmap_filters) {
   uint32_t count = 0;
   for (const auto& filter : bitmap_filters) {
      count += filter->cardinality();
   };
   std::map<std::string, json_value_type> tuple_fields;
   tuple_fields[COUNT_FIELD] = static_cast<int32_t>(count);
   return QueryResult{std::vector<QueryResultEntry>{{tuple_fields}}};
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
         break;
      }
      order_by_definition.push_back(parseOrderByField(order_by_field));
   }
   for (const OrderByField& field : order_by_definition) {
      if (field.name != COUNT_FIELD && !std::any_of(group_by_metadata.begin(), group_by_metadata.end(), [&field](const config::DatabaseMetadata& group_by_field) {
             return group_by_field.name == field.name;
          })) {
         throw QueryParseException(
            "The orderByField '" + field.name +
            "' cannot be ordered by, as it does not appear in the groupByFields."
         );
      }
   }

   std::vector<storage::ColumnGroup> group_by_column_groups;
   for (const auto& partition : database.partitions) {
      group_by_column_groups.emplace_back(partition.columns.getSubgroup(group_by_metadata));
   }

   const size_t tuple_size = getTupleSize(group_by_metadata);

   tbb::enumerable_thread_specific<std::unordered_map<Tuple, uint32_t>> maps;

   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, database.partitions.size()),
      [&](tbb::blocked_range<uint32_t> range) {
         std::unordered_map<Tuple, uint32_t>& map = maps.local();
         for (uint32_t partition_id = range.begin(); partition_id != range.end(); ++partition_id) {
            for (const uint32_t sequence_id : *bitmap_filters[partition_id]) {
               ++map[Tuple(sequence_id, group_by_column_groups[partition_id], tuple_size)];
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
   std::vector<QueryResultEntry> result = generateResult(final_map);
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
