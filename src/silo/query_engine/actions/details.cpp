#include "silo/query_engine/actions/details.h"

#include <algorithm>
#include <random>
#include <ranges>
#include <utility>

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

namespace {

std::vector<silo::storage::ColumnMetadata> parseFields(
   const silo::Database& database,
   const std::vector<std::string>& fields
) {
   if (fields.empty()) {
      return database.columns.metadata;
   }
   std::vector<silo::storage::ColumnMetadata> field_metadata;
   for (const std::string& field : fields) {
      const auto& metadata = database.database_config.getMetadata(field);
      CHECK_SILO_QUERY(metadata.has_value(), "Metadata field " + field + " not found.");
      field_metadata.push_back({metadata->name, metadata->getColumnType()});
   }
   return field_metadata;
}

}  // namespace

namespace silo::query_engine::actions {
Details::Details(std::vector<std::string> fields)
    : fields(std::move(fields)) {}

void Details::validateOrderByFields(const Database& database) const {
   const std::vector<silo::storage::ColumnMetadata> field_metadata = parseFields(database, fields);

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::any_of(
            field_metadata,
            [&](const silo::storage::ColumnMetadata& metadata) {
               return metadata.name == field.name;
            }
         ),
         "OrderByField " + field.name + " is not contained in the result of this operation."
      );
   }
}

QueryResult Details::execute(
   const silo::Database& /*database*/,
   std::vector<CopyOnWriteBitmap> /*bitmap_filter*/
) const {
   return QueryResult{};
}

namespace {
std::vector<actions::Tuple> mergeSortedTuples(
   const Tuple::Comparator& tuple_comparator,
   std::vector<std::vector<actions::Tuple>>& tuples,
   const uint32_t to_produce
) {
   using iterator = std::vector<actions::Tuple>::iterator;
   std::vector<std::pair<iterator, iterator>> min_heap;
   for (auto& tuple_vector : tuples) {
      if (tuple_vector.begin() != tuple_vector.end()) {
         min_heap.emplace_back(tuple_vector.begin(), tuple_vector.end());
      }
   }

   auto heap_cmp =
      [&](const std::pair<iterator, iterator>& lhs, const std::pair<iterator, iterator>& rhs) {
         return tuple_comparator(*rhs.first, *lhs.first);
      };
   std::ranges::make_heap(min_heap, heap_cmp);

   std::vector<actions::Tuple> result;

   for (uint32_t counter = 0; counter < to_produce && !min_heap.empty(); counter++) {
      std::ranges::pop_heap(min_heap, heap_cmp);
      auto& current = min_heap.back();
      result.emplace_back(std::move(*current.first++));
      if (current.first == current.second) {
         min_heap.pop_back();
      } else {
         std::ranges::push_heap(min_heap, heap_cmp);
      }
   }

   return result;
}

std::vector<actions::Tuple> produceSortedTuplesWithLimit(
   std::vector<TupleFactory>& tuple_factories,
   std::vector<CopyOnWriteBitmap>& bitmap_filter,
   const Tuple::Comparator tuple_comparator,
   const uint32_t to_produce
) {
   std::vector<std::vector<actions::Tuple>> tuples_per_partition(bitmap_filter.size());
   tbb::parallel_for(tbb::blocked_range<size_t>(0U, bitmap_filter.size()), [&](auto local) {
      for (size_t partition_id = local.begin(); partition_id != local.end(); partition_id++) {
         const auto& bitmap = bitmap_filter.at(partition_id);
         TupleFactory& tuple_factory = tuple_factories.at(partition_id);
         std::vector<actions::Tuple>& my_tuples = tuples_per_partition.at(partition_id);
         const size_t result_size =
            std::min(bitmap->cardinality(), static_cast<uint64_t>(to_produce));
         my_tuples = tuple_factory.allocateMany(result_size);
         auto iterator = bitmap->begin();
         auto end = bitmap->end();
         uint32_t counter = 0;
         for (; iterator != end && counter < to_produce; iterator++) {
            tuple_factory.overwrite(my_tuples.at(counter), *iterator);
            counter++;
         }

         if (iterator != end) {
            std::ranges::make_heap(my_tuples, tuple_comparator);
            Tuple current_tuple = tuple_factory.allocateOne(*iterator);
            if (tuple_comparator(current_tuple, my_tuples.front())) {
               std::ranges::pop_heap(my_tuples, tuple_comparator);
               my_tuples.back() = current_tuple;
               std::ranges::push_heap(my_tuples, tuple_comparator);
            }
            iterator++;
            for (; iterator != end; iterator++) {
               tuple_factory.overwrite(current_tuple, *iterator);
               if (tuple_comparator(current_tuple, my_tuples.front())) {
                  std::ranges::pop_heap(my_tuples, tuple_comparator);
                  my_tuples.back() = current_tuple;
                  std::ranges::push_heap(my_tuples, tuple_comparator);
               }
            }
            std::ranges::sort_heap(my_tuples, tuple_comparator);
         } else {
            std::ranges::sort(my_tuples, tuple_comparator);
         }
      }
   });
   return mergeSortedTuples(tuple_comparator, tuples_per_partition, to_produce);
}

std::vector<Tuple> produceAllTuples(
   std::vector<TupleFactory>& tuple_factories,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) {
   if (tuple_factories.empty()) {
      return {};
   }

   std::vector<uint64_t> offsets(bitmap_filter.size() + 1);
   for (size_t partition_id = 0; partition_id != bitmap_filter.size(); partition_id++) {
      offsets[partition_id + 1] =
         offsets[partition_id] + bitmap_filter.at(partition_id)->cardinality();
   }

   std::vector<Tuple> all_tuples = tuple_factories.front().allocateMany(offsets.back());

   tbb::parallel_for(tbb::blocked_range<size_t>(0U, bitmap_filter.size()), [&](auto local) {
      for (size_t partition_id = local.begin(); partition_id != local.end(); partition_id++) {
         auto& tuple_factory = tuple_factories.at(partition_id);
         const auto& bitmap = bitmap_filter.at(partition_id);

         auto cursor = all_tuples.begin() +
                       static_cast<decltype(all_tuples)::difference_type>(offsets.at(partition_id));
         for (const uint32_t sequence_id : *bitmap) {
            tuple_factory.overwrite(*cursor, sequence_id);
            cursor++;
         }
      }
   });
   return all_tuples;
}
}  // namespace

QueryResult Details::executeAndOrder(
   const silo::Database& database,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   validateOrderByFields(database);
   const std::vector<storage::ColumnMetadata> field_metadata = parseFields(database, fields);

   std::vector<TupleFactory> tuple_factories;
   tuple_factories.reserve(database.partitions.size());
   for (const auto& partition : database.partitions) {
      tuple_factories.emplace_back(partition->columns, field_metadata);
   }

   std::vector<actions::Tuple> tuples;
   if (limit.has_value()) {
      tuples = produceSortedTuplesWithLimit(
         tuple_factories,
         bitmap_filter,
         Tuple::getComparator(field_metadata, order_by_fields, randomize_seed),
         limit.value() + offset.value_or(0)
      );
   } else {
      tuples = produceAllTuples(tuple_factories, bitmap_filter);
      if (!order_by_fields.empty() || randomize_seed) {
         std::ranges::sort(
            tuples, Tuple::getComparator(field_metadata, order_by_fields, randomize_seed)
         );
      }
   }

   QueryResult results_in_format;
   for (const auto& tuple : tuples) {
      results_in_format.entriesMut().push_back({tuple.getFields()});
   }
   applyOffsetAndLimit(results_in_format);
   return results_in_format;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action) {
   const std::vector<std::string> fields = json.value("fields", std::vector<std::string>());
   action = std::make_unique<Details>(fields);
}

}  // namespace silo::query_engine::actions
