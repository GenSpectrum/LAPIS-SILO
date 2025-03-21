#include "silo/query_engine/actions/aggregated.h"

#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>

#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/actions/tuple.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column_group.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::query_engine::QueryResult;
using silo::query_engine::QueryResultEntry;
using silo::query_engine::actions::Tuple;

namespace {

std::vector<silo::storage::ColumnMetadata> parseGroupByFields(
   const silo::Database& database,
   const std::vector<std::string>& group_by_fields
) {
   std::vector<silo::storage::ColumnMetadata> group_by_metadata;
   for (const std::string& group_by_field : group_by_fields) {
      const auto& metadata = database.database_config.getMetadata(group_by_field);
      CHECK_SILO_QUERY(
         metadata.has_value(), "Metadata field '" + group_by_field + "' to group by not found"
      );
      group_by_metadata.push_back({metadata->name, metadata->getColumnType()});
   }
   return group_by_metadata;
}

const std::string COUNT_FIELD = "count";

std::vector<QueryResultEntry> generateResult(std::unordered_map<Tuple, uint32_t>& tuple_counts) {
   std::vector<QueryResultEntry> result;
   result.reserve(tuple_counts.size());
   for (auto& [tuple, count] : tuple_counts) {
      std::map<std::string, silo::common::JsonValueType> fields = tuple.getFields();
      fields[COUNT_FIELD] = static_cast<int32_t>(count);
      result.push_back({fields});
   }
   return result;
}

QueryResult aggregateWithoutGrouping(const std::vector<CopyOnWriteBitmap>& bitmap_filters) {
   uint32_t count = 0;
   for (const auto& filter : bitmap_filters) {
      count += filter->cardinality();
   }
   std::map<std::string, silo::common::JsonValueType> tuple_fields;
   tuple_fields[COUNT_FIELD] = static_cast<int32_t>(count);
   return QueryResult::fromVector(std::vector<QueryResultEntry>{{tuple_fields}});
}

}  // namespace

namespace silo::query_engine::actions {

Aggregated::Aggregated(std::vector<std::string> group_by_fields)
    : group_by_fields(std::move(group_by_fields)) {}

void Aggregated::validateOrderByFields(const Database& database) const {
   const std::vector<silo::storage::ColumnMetadata> field_metadata =
      parseGroupByFields(database, group_by_fields);

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == COUNT_FIELD || std::ranges::any_of(
                                         field_metadata,
                                         [&](const silo::storage::ColumnMetadata& metadata) {
                                            return metadata.name == field.name;
                                         }
                                      ),
         "The orderByField '" + field.name +
            "' cannot be ordered by, as it does not appear in the groupByFields."
      );
   }
}

QueryResult Aggregated::execute(
   const Database& database,
   std::vector<CopyOnWriteBitmap> bitmap_filters
) const {
   if (group_by_fields.empty()) {
      return aggregateWithoutGrouping(bitmap_filters);
   }
   // TODO(#133) optimize when equal to partition_by field
   // TODO(#133) optimize single field groupby

   const std::vector<silo::storage::ColumnMetadata> group_by_metadata =
      parseGroupByFields(database, group_by_fields);

   std::vector<std::unordered_map<Tuple, uint32_t>> tuple_maps;
   std::vector<TupleFactory> tuple_factories;

   for (const auto& partition : database.partitions) {
      tuple_maps.emplace_back();
      tuple_factories.emplace_back(partition.columns, group_by_metadata);
   }

   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, database.partitions.size()),
      [&](tbb::blocked_range<uint32_t> range) {
         for (uint32_t partition_id = range.begin(); partition_id != range.end(); ++partition_id) {
            TupleFactory& tuple_factory = tuple_factories.at(partition_id);
            std::unordered_map<Tuple, uint32_t>& map = tuple_maps.at(partition_id);
            CopyOnWriteBitmap& bitmap = bitmap_filters[partition_id];

            auto iterator = bitmap->begin();
            auto end = bitmap->end();
            if (iterator != end) {
               Tuple current_tuple = tuple_factory.allocateOne(*iterator);
               map.emplace(tuple_factory.copyTuple(current_tuple), 1);
               iterator++;
               for (; iterator != end; iterator++) {
                  tuple_factory.overwrite(current_tuple, *iterator);
                  if (map.contains(current_tuple)) {
                     ++map.at(current_tuple);
                  } else {
                     map.emplace(tuple_factory.copyTuple(current_tuple), 1);
                  }
               }
            }
         }
      }
   );
   std::unordered_map<Tuple, uint32_t> final_map;
   for (uint32_t partition_id = 0; partition_id != database.partitions.size(); ++partition_id) {
      auto& tuple_factory = tuple_factories.at(partition_id);
      auto& map = tuple_maps.at(partition_id);
      for (auto& [tuple, value] : map) {
         if (final_map.contains(tuple)) {
            final_map.at(tuple) += value;
         } else {
            final_map.emplace(tuple_factory.copyTuple(tuple), value);
         }
      }
   }
   return QueryResult::fromVector(generateResult(final_map));
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action) {
   const std::vector<std::string> group_by_fields =
      json.value("groupByFields", std::vector<std::string>());
   action = std::make_unique<Aggregated>(group_by_fields);
}

}  // namespace silo::query_engine::actions
