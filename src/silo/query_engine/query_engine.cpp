#include "silo/query_engine/query_engine.h"

#include <tbb/parallel_for.h>
#include <memory>
#include <roaring/roaring.hh>
#include <string>
#include <vector>

// query_parse_exception.h must be before the RAPIDJSON_ASSERT because it is used there
#include "silo/query_engine/query_parse_exception.h"
// Do not remove the next line. It overwrites the rapidjson abort, so it can throw an exception and
// does not abort.
#define RAPIDJSON_ASSERT(x)                                                    \
   if (!(x))                                                                   \
   throw silo::QueryParseException(                                            \
      "The query was not a valid JSON: " + std::string(RAPIDJSON_STRINGIFY(x)) \
   )
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

#include "external/PerfEvent.hpp"
#include "silo/common/log.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/query_result.h"

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace silo {

using roaring::Roaring;

QueryEngine::QueryEngine(const silo::Database& database)
    : database(database) {}

response::QueryResult QueryEngine::executeQuery(const std::string& query) const {
   return ::silo::executeQuery(database, query);
}

// TODO(someone): reduce cognitive complexity
// no linting recursion: is inherent to algorithm
// NOLINTNEXTLINE(readability-function-cognitive-complexity, misc-no-recursion)
std::unique_ptr<BoolExpression> parseExpression(
   const Database& database,
   const rapidjson::Value& json_value,
   int exact
) {
   CHECK_SILO_QUERY(
      json_value.HasMember("type"), "The field 'type' is required in 'filterExpression'"
   );
   CHECK_SILO_QUERY(
      json_value["type"].IsString(), "The field 'type' in 'filterExpression' needs to be a string"
   );
   const std::string expression_type = json_value["type"].GetString();
   if (expression_type == "True") {
      return std::make_unique<FullExpression>();
   }
   if (expression_type == "And") {
      auto result = std::make_unique<AndExpression>();
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an And expression"
      );
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an And expression needs to be an array"
      );
      std::transform(
         json_value["children"].GetArray().begin(), json_value["children"].GetArray().end(),
         std::back_inserter(result->children),
         [&](const rapidjson::Value& value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, value, exact);
         }
      );
      return result;
   }
   if (expression_type == "Or") {
      auto result = std::make_unique<OrExpression>();
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an Or expression"
      );
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an Or expression needs to be an array"
      );
      std::transform(
         json_value["children"].GetArray().begin(), json_value["children"].GetArray().end(),
         std::back_inserter(result->children),
         [&](const rapidjson::Value& value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, value, exact);
         }
      );
      return result;
   }
   if (expression_type == "N-Of") {
      CHECK_SILO_QUERY(
         json_value.HasMember("children"), "The field 'children' is required in an N-Of expression"
      );
      CHECK_SILO_QUERY(
         json_value["children"].IsArray(),
         "The field 'children' in an N-Of expression needs to be an array"
      );
      CHECK_SILO_QUERY(
         json_value.HasMember("numberOfMatchers"),
         "The field 'numberOfMatchers' is required in an N-Of expression"
      );
      CHECK_SILO_QUERY(
         json_value["numberOfMatchers"].IsUint(),
         "The field 'numberOfMatchers' in an N-Of expression needs to be an unsigned integer"
      );

      auto result = std::make_unique<NOfExpression>(
         json_value["numberOfMatchers"].GetUint(), json_value["matchExactly"].GetBool()
      );
      std::transform(
         json_value["children"].GetArray().begin(), json_value["children"].GetArray().end(),
         std::back_inserter(result->children),
         [&](const rapidjson::Value& json_value) {  // NOLINT(misc-no-recursion)
            return parseExpression(database, json_value, exact);
         }
      );
      if (json_value.HasMember("implementation") && json_value["implementation"].IsUint()) {
         const std::string implementation_name = json_value["implementation"].GetString();
         if (implementation_name == "generic") {
            result->implementation = NOfExpressionImplementation::GENERIC;
         } else if (implementation_name == "LOOP_DATABASE_PARTITION") {
            result->implementation = NOfExpressionImplementation::LOOP_DATABASE_PARTITION;
         } else if (implementation_name == "N_WAY_HEAP_MERGE") {
            result->implementation = NOfExpressionImplementation::N_WAY_HEAP_MERGE;
         } else {
            throw QueryParseException(
               "Unknown implementation for NOfExpression: " + implementation_name
            );
         }
      }
      return result;
   }
   if (expression_type == "Not") {
      auto result = std::make_unique<NegatedExpression>();
      result->child = parseExpression(database, json_value["child"], -exact);
      return result;
   }
   if (expression_type == "DateBetween") {
      auto result = std::make_unique<DateBetweenExpression>();
      if (json_value["from"].IsNull()) {
         result->open_from = true;
      } else {
         result->open_from = false;

         struct std::tm time_object {};
         std::istringstream date_from_stream(json_value["from"].GetString());
         date_from_stream >> std::get_time(&time_object, "%Y-%m-%d");
         result->date_from = mktime(&time_object);
      }

      if (json_value["to"].IsNull()) {
         result->open_to = true;
      } else {
         result->open_to = false;

         struct std::tm time_object {};
         std::istringstream date_to_stream(json_value["to"].GetString());
         date_to_stream >> std::get_time(&time_object, "%Y-%m-%d");
         result->date_to = mktime(&time_object);
      }
      return result;
   }
   if (expression_type == "NucleotideEquals") {
      CHECK_SILO_QUERY(
         json_value.HasMember("position"),
         "The field 'position' is required in a NucleotideEquals expression"
      )
      CHECK_SILO_QUERY(
         json_value["position"].GetUint(),
         "The field 'position' in a NucleotideEquals expression needs to be an unsigned "
         "integer"
      );
      const unsigned position = json_value["position"].GetUint();
      const std::string& nucleotide_symbol = json_value["symbol"].GetString();
      NUCLEOTIDE_SYMBOL value;
      if (nucleotide_symbol.at(0) == '.') {
         const char character = database.global_reference[0].at(position);
         value = toNucleotideSymbol(character);
      } else {
         value = toNucleotideSymbol(nucleotide_symbol.at(0));
      }
      if (exact >= 0) {
         return std::make_unique<NucleotideSymbolEqualsExpression>(position, value);
      }
      return std::make_unique<NucleotideSymbolMaybeExpression>(position, value);
   }
   if (expression_type == "HasNucleotideMutation") {
      CHECK_SILO_QUERY(
         json_value.HasMember("position"),
         "The field 'position' is required in a HasNucleotideMutation expression"
      )
      CHECK_SILO_QUERY(
         json_value["position"].GetUint(),
         "The field 'position' in a HasNucleotideMutation expression needs to be an unsigned "
         "integer"
      );
      const unsigned position = json_value["position"].GetUint();
      const char ref_symbol = database.global_reference[0].at(position);
      /// this <= is correct! the negation would flip the exact bit from -1 to +1 and vice versa
      if (exact > 0) {  /// NucleotideSymbolMaybeExpression
         return std::make_unique<NegatedExpression>(
            std::make_unique<NucleotideSymbolMaybeExpression>(
               position, silo::toNucleotideSymbol(ref_symbol)
            )
         );
      }
      return std::make_unique<NegatedExpression>(std::make_unique<NucleotideSymbolEqualsExpression>(
         position, silo::toNucleotideSymbol(ref_symbol)
      ));
   }
   if (expression_type == "PangoLineage") {
      const bool include_sublineages = json_value["include_sublineages"].GetBool();
      std::string lineage = json_value["value"].GetString();
      std::transform(lineage.begin(), lineage.end(), lineage.begin(), ::toupper);
      lineage = resolvePangoLineageAlias(database.getAliasKey(), lineage);
      const uint32_t lineage_key = database.dict->getPangoLineageIdInLookup(lineage);
      return std::make_unique<PangoLineageExpression>(lineage_key, include_sublineages);
   }
   if (expression_type == "StringEquals") {
      const std::string& column = json_value["column"].GetString();
      if (column == "country") {
         return std::make_unique<CountryExpression>(
            database.dict->getCountryIdInLookup(json_value["value"].GetString())
         );
      }
      if (column == "region") {
         return std::make_unique<RegionExpression>(
            database.dict->getRegionIdInLookup(json_value["value"].GetString())
         );
      }
      const uint32_t column_key =
         database.dict->getColumnIdInLookup(json_value["column"].GetString());
      const uint32_t value_key =
         database.dict->getPangoLineageIdInLookup(json_value["value"].GetString());
      return std::make_unique<StringEqualsExpression>(column_key, value_key);
   }
   if (expression_type == "Maybe") {
      auto result = std::make_unique<NegatedExpression>();
      result->child = parseExpression(database, json_value["child"], -1);
      return result;
   }
   if (expression_type == "Exact") {
      auto result = std::make_unique<NegatedExpression>();
      result->child = parseExpression(database, json_value["child"], 1);
      return result;
   }
   throw QueryParseException("Unknown object filter type '" + expression_type + "'");
}

BooleanExpressionResult AndExpression::evaluate(
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<BooleanExpressionResult> children_bm;
   children_bm.reserve(children.size());
   std::transform(
      children.begin(), children.end(), std::back_inserter(children_bm),
      [&](const auto& child) { return child->evaluate(database, database_partition); }
   );
   std::vector<BooleanExpressionResult> negated_children_bm;
   negated_children.reserve(negated_children.size());
   std::transform(
      negated_children.begin(), negated_children.end(), std::back_inserter(negated_children_bm),
      [&](const auto& child) { return child->evaluate(database, database_partition); }
   );
   /// Sort ascending, such that intermediate results are kept small
   std::sort(
      children_bm.begin(), children_bm.end(),
      [](const BooleanExpressionResult& expression1, const BooleanExpressionResult& expression2) {
         return expression1.getAsConst()->cardinality() < expression2.getAsConst()->cardinality();
      }
   );

   Roaring* result;
   if (children_bm.empty()) {
      const unsigned size_of_negated_children_bitmap = negated_children_bm.size();
      const Roaring* union_tmp[size_of_negated_children_bitmap];  // NOLINT
      for (unsigned i = 0; i < size_of_negated_children_bitmap; i++) {
         union_tmp[i] = negated_children_bm[i].getAsConst();
      }
      result = new Roaring(Roaring::fastunion(size_of_negated_children_bitmap, union_tmp));
      result->flip(0, database_partition.sequenceCount);
      for (auto& bitmap : negated_children_bm) {
         bitmap.free();
      }
      return {result, nullptr};
   }
   if (children_bm.size() == 1) {
      if (negated_children_bm.empty()) {
         throw std::runtime_error(
            "Error during 'And' evaluation: negated children were empty although their was only "
            "one non-negated child."
         );
      }
      if (children_bm[0].mutable_res) {
         result = children_bm[0].mutable_res;
      } else {
         auto tmp = *children_bm[0].immutable_res - *negated_children_bm[0].getAsConst();
         result = new Roaring(tmp);
      }
      /// Sort negated children descending by size
      std::sort(
         negated_children_bm.begin(), negated_children_bm.end(),
         [](const BooleanExpressionResult& expression_result1,
            const BooleanExpressionResult& expression_result2) {
            return expression_result1.getAsConst()->cardinality() >
                   expression_result2.getAsConst()->cardinality();
         }
      );
      for (auto neg_bm : negated_children_bm) {
         *result -= *neg_bm.getAsConst();
         neg_bm.free();
      }
      return {result, nullptr};
   }
   if (children_bm[0].mutable_res) {
      result = children_bm[0].mutable_res;
      *result &= *children_bm[1].getAsConst();
      children_bm[1].free();
   } else if (children_bm[1].mutable_res) {
      result = children_bm[1].mutable_res;
      *result &= *children_bm[0].getAsConst();
   } else {
      auto bitmap = *children_bm[0].immutable_res & *children_bm[1].immutable_res;
      result = new Roaring(bitmap);
   }
   for (unsigned i = 2; i < children.size(); i++) {
      auto bitmap = children_bm[i];
      *result &= *bitmap.getAsConst();
      bitmap.free();
   }
   /// Sort negated children descending by size
   std::sort(
      negated_children_bm.begin(), negated_children_bm.end(),
      [](const BooleanExpressionResult& expression_result1,
         const BooleanExpressionResult& expression_result2) {
         return expression_result1.getAsConst()->cardinality() >
                expression_result2.getAsConst()->cardinality();
      }
   );
   for (auto neg_bm : negated_children_bm) {
      *result -= *neg_bm.getAsConst();
      neg_bm.free();
   }
   return {result, nullptr};
}

BooleanExpressionResult OrExpression::evaluate(
   const Database& database,
   const DatabasePartition& database_partition
) {
   const unsigned size_of_children = children.size();
   const Roaring* union_tmp[size_of_children];           // NOLINT
   BooleanExpressionResult child_res[size_of_children];  // NOLINT
   for (unsigned i = 0; i < size_of_children; i++) {
      auto tmp = children[i]->evaluate(database, database_partition);
      child_res[i] = tmp;
      union_tmp[i] = tmp.getAsConst();
   }
   auto* result = new Roaring(Roaring::fastunion(children.size(), union_tmp));
   for (unsigned i = 0; i < size_of_children; i++) {
      child_res[i].free();
   }
   return {result, nullptr};
}

BooleanExpressionResult nOfExpressionEvaluateGenericImplementationNoExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   auto* result = new Roaring();
   std::vector<uint16_t> count(database_partition.sequenceCount);
   std::vector<uint32_t> correct;
   for (const auto& child : self->children) {
      auto expression_result = child->evaluate(database, database_partition);
      for (uint32_t const expression_id : *expression_result.getAsConst()) {
         if (++count[expression_id] == self->number_of_matchers) {
            correct.push_back(expression_id);
         }
      }
      expression_result.free();
      if (!correct.empty()) {
         result->addMany(correct.size(), correct.data());
         correct.clear();
      }
   }
   return {result, nullptr};
}

BooleanExpressionResult nOfExpressionEvaluateGenericImplementationExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<uint16_t> count;
   std::vector<uint32_t> at_least;
   std::vector<uint32_t> too_much;
   count.resize(database_partition.sequenceCount);
   for (const auto& child : self->children) {
      auto expression_result = child->evaluate(database, database_partition);
      for (uint32_t const expression_id : *expression_result.getAsConst()) {
         ++count[expression_id];
         if (count[expression_id] == self->number_of_matchers + 1) {
            too_much.push_back(expression_id);
         } else if (count[expression_id] == self->number_of_matchers) {
            at_least.push_back(expression_id);
         }
      }
      expression_result.free();
   }
   /// Sort because set_difference needs sorted vectors
   std::sort(at_least.begin(), at_least.end());
   std::sort(too_much.begin(), too_much.end());
   std::vector<uint32_t> correct;
   std::set_difference(
      at_least.begin(), at_least.end(), too_much.begin(), too_much.end(),
      std::back_inserter(correct)
   );
   return {new Roaring(correct.size(), correct.data()), nullptr};
}

/// DPLoop
BooleanExpressionResult nOfExpressionEvaluateLoopDatabasePartitionImplementationNoExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<Roaring*> partition_bitmaps(self->number_of_matchers);
   /// Copy bitmap of first child if immutable, otherwise use it directly
   auto tmp = self->children[0]->evaluate(database, database_partition);
   if (tmp.mutable_res) {
      /// Do not need to delete tmp.mutable_res later, because partition_bitmaps[0] will be deleted
      partition_bitmaps[0] = tmp.mutable_res;
   } else {
      partition_bitmaps[0] = new Roaring(*tmp.immutable_res);
   }
   /// Initialize all bitmaps. Delete them later.
   for (unsigned i = 1; i < self->number_of_matchers; ++i) {
      partition_bitmaps[i] = new Roaring();
   }

   for (unsigned i = 1; i < self->children.size(); ++i) {
      auto bitmap = self->children[i]->evaluate(database, database_partition);
      /// positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the
      /// conjunction would return 0
      for (unsigned j = std::min(self->number_of_matchers - 1, i); j >= 1; --j) {
         *partition_bitmaps[j] |= *partition_bitmaps[j - 1] & *bitmap.getAsConst();
      }
      *partition_bitmaps[0] |= *bitmap.getAsConst();
      bitmap.free();
   }

   /// Delete all unneeded bitmaps
   for (unsigned i = 0; i < self->number_of_matchers - 1; ++i) {
      delete partition_bitmaps[i];
   }

   return {partition_bitmaps.back(), nullptr};
}

/// DPLoop
BooleanExpressionResult nOfExpressionEvaluateLoopDatabasePartitionImplementationExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<Roaring*> partition_bitmaps(self->number_of_matchers + 1);
   /// Copy bitmap of first child if immutable, otherwise use it directly
   auto tmp = self->children[0]->evaluate(database, database_partition);
   if (tmp.mutable_res) {
      /// Do not need to delete tmp.mutable_res later, because partition_bitmaps[0] will be deleted
      partition_bitmaps[0] = tmp.mutable_res;
   } else {
      partition_bitmaps[0] = new Roaring(*tmp.immutable_res);
   }
   /// Initialize all bitmaps. Delete them later.
   for (unsigned i = 1; i < self->number_of_matchers + 1; ++i) {
      partition_bitmaps[i] = new Roaring();
   }

   for (unsigned i = 1; i < self->children.size(); ++i) {
      auto bitmap = self->children[i]->evaluate(database, database_partition);
      /// positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the
      /// conjunction would return 0
      for (unsigned j = std::min(self->number_of_matchers, i); j >= 1; --j) {
         *partition_bitmaps[j] |= *partition_bitmaps[j - 1] & *bitmap.getAsConst();
      }
      *partition_bitmaps[0] |= *bitmap.getAsConst();
      bitmap.free();
   }

   /// Delete
   for (unsigned i = 0; i < self->number_of_matchers - 1; ++i) {
      delete partition_bitmaps[i];
   }

   /// Because exact, we remove all that have too many
   *partition_bitmaps[self->number_of_matchers - 1] -= *partition_bitmaps[self->number_of_matchers];

   delete partition_bitmaps[self->number_of_matchers];

   return {partition_bitmaps[self->number_of_matchers - 1], nullptr};
}

// N-Way Heap-Merge, for threshold queries
BooleanExpressionResult nOfExpressionEvaluateNWayHeapMergeImplementationNoExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<BooleanExpressionResult> child_maps;
   struct BitmapIterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<BitmapIterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(database, database_partition);
      child_maps.push_back(tmp);
      /// Invariant: All heap members 'cur' field must contain an element that needs to be processed
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end()) {
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
      }
   }

   /// stl heap is max-heap. We want min, therefore we define a greater-than sorter
   /// as opposed to the standard less-than sorter
   auto min_heap_sort = [](const BitmapIterator& iterator1, const BitmapIterator& iterator2) {
      return *iterator1.cur > *iterator2.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);

   auto* result = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = -1;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);
      /// Take element and ensure invariant
      uint32_t const val = *iterator_heap.back().cur;
      iterator_heap.back().cur++;
      if (iterator_heap.back().cur == iterator_heap.back().end) {
         iterator_heap.pop_back();
      } else {
         std::push_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);
      }
      if (val == last_val) {
         cur_count++;
      } else {
         if (cur_count >= self->number_of_matchers) {
            buffer.push_back(last_val);
            if (buffer.size() == BUFFER_SIZE) {
               result->addMany(BUFFER_SIZE, buffer.data());
               buffer.clear();
            }
         }
         last_val = val;
         cur_count = 1;
      }
   }
   if (cur_count >= self->number_of_matchers) {
      buffer.push_back(last_val);
   }

   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }

   for (auto& child_map : child_maps) {
      child_map.free();
   }

   return {result, nullptr};
}

// N-Way Heap-Merge, for exact queries
BooleanExpressionResult nOfExpressionEvaluateNWayHeapMergeImplementationExactMatch(
   const NOfExpression* self,
   const Database& database,
   const DatabasePartition& database_partition
) {
   std::vector<BooleanExpressionResult> child_maps;
   struct BitmapIterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<BitmapIterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(database, database_partition);
      child_maps.push_back(tmp);
      /// Invariant: All heap members 'cur' field must contain an element that needs to be processed
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end()) {
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
      }
   }

   /// stl heap is max-heap. We want min, therefore we define a greater-than sorter
   /// as opposed to the standard less-than sorter
   auto sorter = [](const BitmapIterator& iterator1, const BitmapIterator& iterator2) {
      return *iterator1.cur > *iterator2.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), sorter);

   auto* result = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = UINT32_MAX;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      /// Take element and ensure invariant
      uint32_t const val = *iterator_heap.back().cur;
      iterator_heap.back().cur++;
      if (iterator_heap.back().cur == iterator_heap.back().end) {
         iterator_heap.pop_back();
      } else {
         std::push_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      }
      if (val == last_val) {
         cur_count++;
      } else {
         if (cur_count == self->number_of_matchers) {
            buffer.push_back(last_val);
            if (buffer.size() == BUFFER_SIZE) {
               result->addMany(BUFFER_SIZE, buffer.data());
               buffer.clear();
            }
         }
         last_val = val;
         cur_count = 1;
      }
   }
   if (cur_count == self->number_of_matchers) {
      buffer.push_back(last_val);
   }

   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }

   for (auto& child_map : child_maps) {
      child_map.free();
   }

   return {result, nullptr};
}

BooleanExpressionResult NOfExpression::evaluate(
   const Database& database,
   const DatabasePartition& database_partition
) {
   switch (implementation) {
      case NOfExpressionImplementation::GENERIC:
         if (match_exactly) {
            return nOfExpressionEvaluateGenericImplementationExactMatch(
               this, database, database_partition
            );
         } else {
            return nOfExpressionEvaluateGenericImplementationNoExactMatch(
               this, database, database_partition
            );
         }
      case NOfExpressionImplementation::LOOP_DATABASE_PARTITION:
      default:
         if (match_exactly) {
            return nOfExpressionEvaluateLoopDatabasePartitionImplementationExactMatch(
               this, database, database_partition
            );
         } else {
            return nOfExpressionEvaluateLoopDatabasePartitionImplementationNoExactMatch(
               this, database, database_partition
            );
         }
      case NOfExpressionImplementation::N_WAY_HEAP_MERGE:
         if (match_exactly) {
            return nOfExpressionEvaluateNWayHeapMergeImplementationExactMatch(
               this, database, database_partition
            );
         } else {
            return nOfExpressionEvaluateNWayHeapMergeImplementationNoExactMatch(
               this, database, database_partition
            );
         }
   }
}

BooleanExpressionResult NegatedExpression::evaluate(
   const Database& database,
   const DatabasePartition& database_partition
) {
   auto tmp = child->evaluate(database, database_partition);
   auto* result = tmp.mutable_res ? tmp.mutable_res : new Roaring(*tmp.immutable_res);
   result->flip(0, database_partition.sequenceCount);
   return {result, nullptr};
}

BooleanExpressionResult DateBetweenExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   if (open_from && open_to) {
      auto* result = new Roaring();
      result->addRange(0, database_partition.sequenceCount);
      return {result, nullptr};
   }

   auto* result = new Roaring;
   const auto* base = database_partition.meta_store.sequence_id_to_date.data();
   for (const Chunk& chunk : database_partition.getChunks()) {
      const auto* begin = &database_partition.meta_store.sequence_id_to_date[chunk.offset];
      const auto* end =
         &database_partition.meta_store.sequence_id_to_date[chunk.offset + chunk.count];
      uint32_t const lower =
         open_from ? begin - base : std::lower_bound(begin, end, this->date_from) - base;
      uint32_t const upper =
         open_to ? end - base : std::upper_bound(begin, end, this->date_to) - base;
      result->addRange(lower, upper);
   }
   return {result, nullptr};
}

BooleanExpressionResult DateBetweenExpression::select(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   if (open_from && open_to) {
      return in_filter;
   }
   Roaring* result;
   if (in_filter.mutable_res) {
      result = in_filter.mutable_res;
   } else {
      result = new Roaring(*in_filter.getAsConst());
   }
   const auto* base = database_partition.meta_store.sequence_id_to_date.data();
   uint32_t lower = 0;
   uint32_t upper = 0;
   for (const Chunk& chunk : database_partition.getChunks()) {
      const auto* begin = &database_partition.meta_store.sequence_id_to_date[chunk.offset];
      const auto* end =
         &database_partition.meta_store.sequence_id_to_date[chunk.offset + chunk.count];
      lower = open_from ? begin - base : std::lower_bound(begin, end, this->date_from) - base;
      result->removeRange(upper, lower);
      upper = open_to ? end - base : std::upper_bound(begin, end, this->date_to) - base;
   }
   if (!open_to) {
      result->removeRange(upper, database_partition.sequenceCount);
   }
   return {result, nullptr};
}

BooleanExpressionResult DateBetweenExpression::selectNegated(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   if (open_from && open_to) {
      return in_filter;
   }
   Roaring* result;
   if (in_filter.mutable_res) {
      result = in_filter.mutable_res;
   } else {
      result = new Roaring(*in_filter.getAsConst());
   }
   const auto* base = database_partition.meta_store.sequence_id_to_date.data();
   for (const Chunk& chunk : database_partition.getChunks()) {
      const auto* begin = &database_partition.meta_store.sequence_id_to_date[chunk.offset];
      const auto* end =
         &database_partition.meta_store.sequence_id_to_date[chunk.offset + chunk.count];
      uint32_t const lower =
         open_from ? begin - base : std::lower_bound(begin, end, this->date_from) - base;
      uint32_t const upper =
         open_to ? end - base : std::upper_bound(begin, end, this->date_to) - base;
      result->removeRange(lower, upper);
   }
   return {result, nullptr};
}
ExpressionType DateBetweenExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
DateBetweenExpression::DateBetweenExpression() = default;
DateBetweenExpression::DateBetweenExpression(
   time_t date_from,
   bool open_from,
   time_t date_to,
   bool open_to
)
    : date_from(date_from),
      open_from(open_from),
      date_to(date_to),
      open_to(open_to) {}
std::string DateBetweenExpression::toString(const Database& /*database*/) {
   std::string res = "[Date-between ";
   res += (open_from ? "unbound" : std::to_string(date_from));
   res += " and ";
   res += (open_to ? "unbound" : std::to_string(date_to));
   res += "]";
   return res;
}
std::unique_ptr<BoolExpression> DateBetweenExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) const {
   return std::make_unique<DateBetweenExpression>(date_from, open_from, date_to, open_to);
}

BooleanExpressionResult NucleotideSymbolEqualsExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   return {nullptr, database_partition.seq_store.getBitmap(position, value)};
}

BooleanExpressionResult NucleotideSymbolMaybeExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   if (!negated) {
      /// Normal case
      return {database_partition.seq_store.getBitmapFromAmbiguousSymbol(position, value), nullptr};
   }
   /// The bitmap of this->value has been flipped... still have to union it with the other
   /// symbols
   return {
      database_partition.seq_store.getFlippedBitmapFromAmbiguousSymbol(position, value), nullptr};
}

BooleanExpressionResult PangoLineageExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   if (lineageKey == UINT32_MAX) {
      return {new Roaring(), nullptr};
   }
   if (include_sublineages) {
      return {nullptr, &database_partition.meta_store.sublineage_bitmaps[lineageKey]};
   }
   return {nullptr, &database_partition.meta_store.lineage_bitmaps[lineageKey]};
}

BooleanExpressionResult CountryExpression::evaluate(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) {
   return {nullptr, &database_partition.meta_store.country_bitmaps[country_key]};
}

BooleanExpressionResult RegionExpression::evaluate(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) {
   return {nullptr, &database_partition.meta_store.region_bitmaps[region_key]};
}

BooleanExpressionResult PositionHasNucleotideSymbolNExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t seq = 0; seq < database_partition.sequenceCount; seq++) {
      if (database_partition.seq_store.nucleotide_symbol_n_bitmaps[seq].contains(position)) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   return {result, nullptr};
}

BooleanExpressionResult PositionHasNucleotideSymbolNExpression::select(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t const sequence : *in_filter.getAsConst()) {
      if (database_partition.seq_store.nucleotide_symbol_n_bitmaps[sequence].contains(position)) {
         buffer.push_back(sequence);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {result, nullptr};
}

BooleanExpressionResult PositionHasNucleotideSymbolNExpression::selectNegated(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t const sequence : *in_filter.getAsConst()) {
      if (!database_partition.seq_store.nucleotide_symbol_n_bitmaps[sequence].contains(position)) {
         buffer.push_back(sequence);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {result, nullptr};
}
ExpressionType PositionHasNucleotideSymbolNExpression::type() const {
   return ExpressionType::FILTER;
}
PositionHasNucleotideSymbolNExpression::PositionHasNucleotideSymbolNExpression(unsigned int position
)
    : position(position) {}
std::string PositionHasNucleotideSymbolNExpression::toString(const Database& /*database*/) {
   std::string res = std::to_string(position) + "N";
   return res;
}
std::unique_ptr<BoolExpression> PositionHasNucleotideSymbolNExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) const {
   return std::make_unique<PositionHasNucleotideSymbolNExpression>(position);
}

BooleanExpressionResult StringEqualsExpression::evaluate(
   const Database& /*db*/,
   const DatabasePartition& database_partition
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t seq = 0; seq < database_partition.sequenceCount; seq++) {
      if (database_partition.meta_store.columns[column][seq] == value) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   return {result, nullptr};
}

BooleanExpressionResult StringEqualsExpression::select(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t const sequence : *in_filter.getAsConst()) {
      if (database_partition.meta_store.columns[column][sequence] == value) {
         buffer.push_back(sequence);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {result, nullptr};
}

BooleanExpressionResult StringEqualsExpression::selectNegated(
   const Database& /*db*/,
   const DatabasePartition& database_partition,
   BooleanExpressionResult in_filter
) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   auto* result = new Roaring();
   for (uint32_t const seq : *in_filter.getAsConst()) {
      if (database_partition.meta_store.columns[column][seq] != value) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            result->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (!buffer.empty()) {
      result->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {result, nullptr};
}
ExpressionType StringEqualsExpression::type() const {
   return ExpressionType::FILTER;
}
StringEqualsExpression::StringEqualsExpression(uint32_t column, uint64_t value)
    : column(column),
      value(value) {}
std::string StringEqualsExpression::toString(const Database& /*database*/) {
   return std::to_string(column).append("=").append(std::to_string(value));
}
std::unique_ptr<BoolExpression> StringEqualsExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) const {
   if (column == UINT32_MAX || value == UINT64_MAX) {
      return std::make_unique<EmptyExpression>();
   }
   return std::make_unique<StringEqualsExpression>(column, value);
}

BooleanExpressionResult FullExpression::evaluate(
   const Database& /*database*/,
   const DatabasePartition& database_partition
) {
   auto* result = new Roaring();
   result->addRange(0, database_partition.sequenceCount);
   return {result, nullptr};
}
ExpressionType FullExpression::type() const {
   return ExpressionType::FULL;
}
std::string FullExpression::toString(const Database& /*database*/) {
   return "TRUE";
}
std::unique_ptr<BoolExpression> FullExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) const {
   return std::make_unique<silo::FullExpression>();
}

BooleanExpressionResult EmptyExpression::evaluate(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) {
   return {new Roaring(), nullptr};
}
ExpressionType EmptyExpression::type() const {
   return ExpressionType::EMPTY;
}
std::string EmptyExpression::toString(const Database& /*database*/) {
   return "FALSE";
}
std::unique_ptr<BoolExpression> EmptyExpression::simplify(
   const Database& /*database*/,
   const DatabasePartition& /*database_partition*/
) const {
   return std::make_unique<silo::EmptyExpression>();
}
const roaring::Roaring* BooleanExpressionResult::getAsConst() const {
   return mutable_res ? mutable_res : immutable_res;
}
void BooleanExpressionResult::free() const {
   delete mutable_res;
}
BoolExpression::BoolExpression() = default;

ExpressionType NucleotideSymbolEqualsExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression() = default;
NucleotideSymbolEqualsExpression::NucleotideSymbolEqualsExpression(
   unsigned int position,
   NUCLEOTIDE_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolEqualsExpression::toString(const Database& /*database*/) {
   std::string res = std::to_string(position) + genomeSymbolRepresentation(value);
   return res;
}

ExpressionType NucleotideSymbolMaybeExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression() = default;
NucleotideSymbolMaybeExpression::NucleotideSymbolMaybeExpression(
   unsigned int position,
   NUCLEOTIDE_SYMBOL value
)
    : position(position),
      value(value) {}
std::string NucleotideSymbolMaybeExpression::toString(const Database& /*database*/) {
   std::string res = "?" + std::to_string(position) + genomeSymbolRepresentation(value);
   return res;
}

ExpressionType PangoLineageExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
PangoLineageExpression::PangoLineageExpression(uint32_t lineage_key, bool include_sublineages)
    : lineageKey(lineage_key),
      include_sublineages(include_sublineages) {}
std::string PangoLineageExpression::toString(const Database& database) {
   std::string res = database.dict->getPangoLineage(lineageKey);
   if (include_sublineages) {
      res += ".*";
   }
   return res;
}

CountryExpression::CountryExpression(uint32_t country_key)
    : country_key(country_key) {}
std::string CountryExpression::toString(const Database& database) {
   std::string res = "Country=" + database.dict->getCountry(country_key);
   return res;
}
ExpressionType CountryExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}

ExpressionType RegionExpression::type() const {
   return ExpressionType::INDEX_FILTER;
}
RegionExpression::RegionExpression(uint32_t regionKey)
    : region_key(regionKey) {}
std::string RegionExpression::toString(const Database& database) {
   std::string res = "Region=" + database.dict->getRegion(region_key);
   return res;
}

AndExpression::AndExpression() = default;
ExpressionType AndExpression::type() const {
   return ExpressionType::AND;
}
std::string AndExpression::toString(const Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += " & ";
      res += child->toString(database);
   }
   for (auto& child : negated_children) {
      res += " &! ";
      res += child->toString(database);
   }
   res += ")";
   return res;
}

OrExpression::OrExpression() = default;

ExpressionType OrExpression::type() const {
   return ExpressionType::OR;
}

std::string OrExpression::toString(const Database& database) {
   std::string res = "(";
   for (auto& child : children) {
      res += child->toString(database);
      res += " | ";
   }
   res += ")";
   return res;
}

NegatedExpression::NegatedExpression() = default;

NegatedExpression::NegatedExpression(std::unique_ptr<BoolExpression> child)
    : child(std::move(child)) {}

std::string NegatedExpression::toString(const Database& database) {
   std::string res = "!" + child->toString(database);
   return res;
}

ExpressionType NegatedExpression::type() const {
   return ExpressionType::NEG;
}

ExpressionType NOfExpression::type() const {
   return ExpressionType::NOF;
}
NOfExpression::NOfExpression(
   unsigned int number_of_matchers,
   bool match_exactly,
   NOfExpressionImplementation implementation
)
    : number_of_matchers(number_of_matchers),
      implementation(implementation),
      match_exactly(match_exactly) {}

std::string NOfExpression::toString(const Database& database) {
   std::string res;
   if (match_exactly) {
      res = "[exactly-" + std::to_string(number_of_matchers) + "-of:";
   } else {
      res = "[" + std::to_string(number_of_matchers) + "-of:";
   }
   for (auto& child : children) {
      res += child->toString(database);
      res += ", ";
   }
   res += "]";
   return res;
}

}  // namespace silo

// TODO(someone): reduce cognitive complexity
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
silo::response::QueryResult silo::executeQuery(
   const silo::Database& database,
   const std::string& query
) {
   rapidjson::Document json_document;
   json_document.Parse(query.c_str());
   if (!json_document.HasMember("filterExpression") || !json_document["filterExpression"].IsObject() ||
       !json_document.HasMember("action") || !json_document["action"].IsObject()) {
      throw QueryParseException("Query json must contain filterExpression and action.");
   }

   std::vector<std::string> simplified_queries(database.partitions.size());

   response::QueryResult query_result;
   std::unique_ptr<BoolExpression> filter;
   {
      BlockTimer const timer(query_result.parse_time);
      filter = parseExpression(database, json_document["filterExpression"], 0);
      SPDLOG_DEBUG("Parsed query: {}", filter->toString(database));
   }

   LOG_PERFORMANCE("Parse: {} microseconds", std::to_string(query_result.parse_time));

   std::vector<silo::BooleanExpressionResult> partition_filters(database.partitions.size());
   {
      BlockTimer const timer(query_result.filter_time);
      tbb::blocked_range<size_t> const range(0, database.partitions.size(), 1);
      tbb::parallel_for(range.begin(), range.end(), [&](const size_t& partition_index) {
         std::unique_ptr<BoolExpression> part_filter =
            filter->simplify(database, database.partitions[partition_index]);
         simplified_queries[partition_index] = part_filter->toString(database);
         partition_filters[partition_index] =
            part_filter->evaluate(database, database.partitions[partition_index]);
      });
   }
   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, simplified_queries[i]);
   }
   LOG_PERFORMANCE("Execution (filter): {} microseconds", std::to_string(query_result.filter_time));

   {
      BlockTimer const timer(query_result.action_time);
      const auto& action = json_document["action"];
      CHECK_SILO_QUERY(
         action.HasMember("type"), "The field 'type' is required on a SILO query action"
      );
      CHECK_SILO_QUERY(
         action["type"].IsString(), "The field 'type' in a SILO query action needs to be a string"
      );
      const auto& action_type = action["type"].GetString();

      if (action.HasMember("groupByFields")) {
         CHECK_SILO_QUERY(
            action["groupByFields"].IsArray(),
            "The field 'type' in a SILO query action needs to be a string"
         );
         std::vector<std::string> group_by_fields;
         for (const auto& field : action["groupByFields"].GetArray()) {
            group_by_fields.emplace_back(field.GetString());
         }

         if (strcmp(action_type, "Aggregated") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Aggregated is not properly implemented yet"};
         } else if (strcmp(action_type, "List") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::List is not properly implemented yet"};
         } else if (strcmp(action_type, "Mutations") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Mutations is not properly implemented yet"};
         } else {
            query_result.query_result =
               response::ErrorResult{"groupByFields is not properly implemented yet"};
         }

      } else {
         if (strcmp(action_type, "Aggregated") == 0) {
            const unsigned count = executeCount(database, partition_filters);
            query_result.query_result = response::AggregationResult{count};
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
            double min_proportion = DEFAULT_MINIMAL_PROPORTION;
            if (action.HasMember("minProportion") && action["minProportion"].IsDouble()) {
               if (action["minProportion"].GetDouble() <= 0.0) {
                  query_result.query_result = response::ErrorResult{
                     "Invalid proportion", "minProportion must be in interval (0.0,1.0]"};
                  return query_result;
               }
               min_proportion = action["minProportion"].GetDouble();
            }
            std::vector<MutationProportion> mutations =
               executeMutations(database, partition_filters, min_proportion);

            std::vector<response::MutationProportion> output_mutation_proportions(mutations.size());
            std::transform(
               mutations.begin(), mutations.end(), output_mutation_proportions.begin(),
               [](MutationProportion mutation_proportion) {
                  return response::MutationProportion{
                     mutation_proportion.mutation_from +
                        std::to_string(mutation_proportion.position) +
                        mutation_proportion.mutation_to,
                     mutation_proportion.proportion, mutation_proportion.count};
               }
            );
            query_result.query_result = output_mutation_proportions;
         } else {
            query_result.query_result = response::ErrorResult{
               "Unknown action", std::string(action_type) + " is not a valid action"};
         }
      }
   }

   LOG_PERFORMANCE("Execution (action): {} microseconds", std::to_string(query_result.action_time));

   return query_result;
}
