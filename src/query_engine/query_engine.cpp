

#include "silo/query_engine/query_engine.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_for_each.h"
#include <silo/common/PerfEvent.hpp>
#include <syncstream>
#include <vector>

#define RAPIDJSON_ASSERT(x) \
   if (!(x)) throw silo::QueryParseException("The query was not a valid JSON: " + std::string(RAPIDJSON_STRINGIFY(x)))
#include "rapidjson/document.h"

namespace silo {

using roaring::Roaring;

std::unique_ptr<BoolExpression> parse_expression(const Database& db, const rapidjson::Value& js, int exact) {
   assert(js.HasMember("type"));
   assert(js["type"].IsString());
   std::string type = js["type"].GetString();
   if (type == "And") {
      auto ret = std::make_unique<AndEx>();
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return parse_expression(db, js, exact); });
      return ret;
   } else if (type == "Or") {
      auto ret = std::make_unique<OrEx>();
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return parse_expression(db, js, exact); });
      return ret;
   } else if (type == "N-Of") {
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      assert(js.HasMember("n"));
      assert(js["n"].IsUint());

      auto ret = std::make_unique<NOfEx>(js["n"].GetUint(), js["exactly"].GetBool());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return parse_expression(db, js, exact); });
      if (js.HasMember("impl") && js["impl"].IsUint()) {
         ret->impl = js["impl"].GetUint();
      }
      return ret;
   } else if (type == "Neg") {
      auto ret = std::make_unique<NegEx>();
      ret->child = parse_expression(db, js["child"], -exact);
      return ret;
   } else if (type == "DateBetw") {
      auto ret = std::make_unique<DateBetwEx>();
      if (js["from"].IsNull()) {
         ret->open_from = true;
      } else {
         ret->open_from = false;

         struct std::tm tm {};
         std::istringstream ss(js["from"].GetString());
         ss >> std::get_time(&tm, "%Y-%m-%d");
         ret->from = mktime(&tm);
      }

      if (js["to"].IsNull()) {
         ret->open_to = true;
      } else {
         ret->open_to = false;

         struct std::tm tm {};
         std::istringstream ss(js["to"].GetString());
         ss >> std::get_time(&tm, "%Y-%m-%d");
         ret->to = mktime(&tm);
      }
      return ret;
   } else if (type == "NucEq") {
      unsigned position = js["position"].GetUint();
      const std::string& s = js["value"].GetString();
      Symbol value;
      if (s.at(0) == '.') {
         char c = db.global_reference[0].at(position);
         value = to_symbol(c);
      } else {
         value = to_symbol(s.at(0));
      }
      if (exact >= 0) {
         return std::make_unique<NucEqEx>(position, value);
      } else { // Approximate query!
         return std::make_unique<NucMbEx>(position, value);
      }
   } else if (type == "NucMut") {
      assert(js.HasMember("position"));
      unsigned pos = js["position"].GetUint();
      char ref_symbol = db.global_reference[0].at(pos);
      /// this <= is correct! the negation would flip the exact bit from -1 to +1 and vice versa
      if (exact <= 0) { /// NucEqEx
         return std::make_unique<NegEx>(std::make_unique<NucEqEx>(pos, silo::to_symbol(ref_symbol)));
      } else { /// NucMbEx
         return std::make_unique<NegEx>(std::make_unique<NucMbEx>(pos, silo::to_symbol(ref_symbol)));
      }
   } else if (type == "PangoLineage") {
      bool includeSubLineages = js["includeSubLineages"].GetBool();
      std::string lineage = js["value"].GetString();
      std::transform(lineage.begin(), lineage.end(), lineage.begin(), ::toupper);
      lineage = resolve_alias(db.getAliasKey(), lineage);
      uint32_t lineageKey = db.dict->get_pangoid(lineage);
      return std::make_unique<PangoLineageEx>(lineageKey, includeSubLineages);
   } else if (type == "StrEq") {
      const std::string& col = js["column"].GetString();
      if (col == "country") {
         return std::make_unique<CountryEx>(db.dict->get_countryid(js["value"].GetString()));
      } else if (col == "region") {
         return std::make_unique<RegionEx>(db.dict->get_regionid(js["value"].GetString()));
      } else {
         uint32_t colKey = db.dict->get_colid(js["column"].GetString());
         uint32_t valueKey = db.dict->get_pangoid(js["value"].GetString());
         return std::make_unique<StrEqEx>(colKey, valueKey);
      }
   } else if (type == "Maybe") {
      auto ret = std::make_unique<NegEx>();
      ret->child = parse_expression(db, js["child"], -1);
      return ret;
   } else if (type == "Exact") {
      auto ret = std::make_unique<NegEx>();
      ret->child = parse_expression(db, js["child"], 1);
      return ret;
   } else {
      throw QueryParseException("Unknown object type");
   }
}

filter_t AndEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   std::vector<filter_t> children_bm;
   children_bm.reserve(children.size());
   std::transform(children.begin(), children.end(), std::back_inserter(children_bm),
                  [&](const auto& child) { return child->evaluate(db, dbp); });
   std::vector<filter_t> negated_children_bm;
   negated_children.reserve(negated_children.size());
   std::transform(negated_children.begin(), negated_children.end(), std::back_inserter(negated_children_bm),
                  [&](const auto& child) { return child->evaluate(db, dbp); });
   /// Sort ascending, such that intermediate results are kept small
   std::sort(children_bm.begin(), children_bm.end(),
             [](const filter_t& a, const filter_t& b) { return a.getAsConst()->cardinality() < b.getAsConst()->cardinality(); });

   Roaring* ret;
   if (children_bm.empty()) {
      const unsigned n = negated_children_bm.size();
      const Roaring* union_tmp[n];
      for (unsigned i = 0; i < n; i++) {
         union_tmp[i] = negated_children_bm[i].getAsConst();
      }
      ret = new Roaring(Roaring::fastunion(n, union_tmp));
      ret->flip(0, dbp.sequenceCount);
      for (auto& bm : negated_children_bm) {
         bm.free();
      }
      return {ret, nullptr};
   } else if (children_bm.size() == 1) {
      assert(negated_children_bm.size() >= 1);
      if (children_bm[0].mutable_res) {
         ret = children_bm[0].mutable_res;
      } else {
         auto tmp = *children_bm[0].immutable_res - *negated_children_bm[0].getAsConst();
         ret = new Roaring(tmp);
      }
      /// Sort negated children descending by size
      std::sort(negated_children_bm.begin(), negated_children_bm.end(),
                [](const filter_t& a, const filter_t& b) { return a.getAsConst()->cardinality() > b.getAsConst()->cardinality(); });
      for (auto neg_bm : negated_children_bm) {
         *ret -= *neg_bm.getAsConst();
         neg_bm.free();
      }
      return {ret, nullptr};
   } else {
      if (children_bm[0].mutable_res) {
         ret = children_bm[0].mutable_res;
         *ret &= *children_bm[1].getAsConst();
         children_bm[1].free();
      } else if (children_bm[1].mutable_res) {
         ret = children_bm[1].mutable_res;
         *ret &= *children_bm[0].getAsConst();
      } else {
         auto x = *children_bm[0].immutable_res & *children_bm[1].immutable_res;
         ret = new Roaring(x);
      }
      for (unsigned i = 2; i < children.size(); i++) {
         auto bm = children_bm[i];
         *ret &= *bm.getAsConst();
         bm.free();
      }
      /// Sort negated children descending by size
      std::sort(negated_children_bm.begin(), negated_children_bm.end(),
                [](const filter_t& a, const filter_t& b) { return a.getAsConst()->cardinality() > b.getAsConst()->cardinality(); });
      for (auto neg_bm : negated_children_bm) {
         *ret -= *neg_bm.getAsConst();
         neg_bm.free();
      }
      return {ret, nullptr};
   }
}

filter_t OrEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   unsigned n = children.size();
   const Roaring* union_tmp[n];
   filter_t child_res[n];
   for (unsigned i = 0; i < n; i++) {
      auto tmp = children[i]->evaluate(db, dbp);
      child_res[i] = tmp;
      union_tmp[i] = tmp.getAsConst();
   }
   Roaring* ret = new Roaring(Roaring::fastunion(children.size(), union_tmp));
   for (unsigned i = 0; i < n; i++) {
      child_res[i].free();
   }
   return {ret, nullptr};
}

inline void vec_and_not(std::vector<uint32_t>& dest, const std::vector<uint32_t>& v1, const std::vector<uint32_t>& v2) {
   std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(dest));
}

filter_t NOfEx_evaluateImpl0_threshold(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   Roaring* ret = new Roaring();
   std::vector<uint16_t> count(dbp.sequenceCount);
   std::vector<uint32_t> correct;
   for (auto& child : self->children) {
      auto bm = child->evaluate(db, dbp);
      for (uint32_t id : *bm.getAsConst()) {
         if (++count[id] == self->n) {
            correct.push_back(id);
         }
      }
      bm.free();
      if (!correct.empty()) {
         ret->addMany(correct.size(), &correct[0]);
         correct.clear();
      }
   }
   return {ret, nullptr};
}

filter_t NOfEx_evaluateImpl0_exact(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<uint16_t> count;
   std::vector<uint32_t> at_least;
   std::vector<uint32_t> too_much;
   count.resize(dbp.sequenceCount);
   for (auto& child : self->children) {
      auto bm = child->evaluate(db, dbp);
      for (uint32_t id : *bm.getAsConst()) {
         ++count[id];
         if (count[id] == self->n + 1) {
            too_much.push_back(id);
         } else if (count[id] == self->n) {
            at_least.push_back(id);
         }
      }
      bm.free();
   }
   /// Sort because set_difference needs sorted vectors
   std::sort(at_least.begin(), at_least.end());
   std::sort(too_much.begin(), too_much.end());
   std::vector<uint32_t> correct;
   vec_and_not(correct, at_least, too_much);
   return {new Roaring(correct.size(), &correct[0]), nullptr};
}

filter_t NOfEx_evaluateImpl0b_exact(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   Roaring* ret = new Roaring();
   Roaring* too_much = new Roaring();
   std::vector<uint16_t> count;
   std::vector<uint32_t> correct_buffer;
   std::vector<uint32_t> too_much_buffer;
   for (auto& child : self->children) {
      auto bm = child->evaluate(db, dbp);
      for (uint32_t id : *bm.getAsConst()) {
         ++count[id];
         if (count[id] == self->n) {
            correct_buffer.push_back(id);
         } else if (count[id] == self->n + 1) {
            too_much_buffer.push_back(id);
         }
      }
      bm.free();
      if (!correct_buffer.empty()) {
         ret->addMany(correct_buffer.size(), &correct_buffer[0]);
         correct_buffer.clear();
      }
      if (!too_much_buffer.empty()) {
         too_much->addMany(too_much_buffer.size(), &too_much_buffer[0]);
         too_much_buffer.clear();
      }
   }
   *ret -= *too_much;
   delete too_much;
   return {ret, nullptr};
}

/// DPLoop
filter_t NOfEx_evaluateImpl1_threshold(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<Roaring*> dp(self->n);
   /// Copy bm of first child if immutable, otherwise use it directly
   auto tmp = self->children[0]->evaluate(db, dbp);
   if (tmp.mutable_res) {
      /// Do not need to delete tmp.mutable_res later, because dp[0] will be deleted
      dp[0] = tmp.mutable_res;
   } else {
      dp[0] = new Roaring(*tmp.immutable_res);
   }
   /// Initialize all bitmaps. Delete them later.
   for (unsigned i = 1; i < self->n; ++i)
      dp[i] = new Roaring();

   for (unsigned i = 1; i < self->children.size(); ++i) {
      auto bm = self->children[i]->evaluate(db, dbp);
      /// positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the conjunction would return 0
      for (unsigned j = std::min(self->n - 1, i); j >= 1; --j) {
         *dp[j] |= *dp[j - 1] & *bm.getAsConst();
      }
      *dp[0] |= *bm.getAsConst();
      bm.free();
   }

   /// Delete all unneeded bitmaps
   for (unsigned i = 0; i < self->n - 1; ++i)
      delete dp[i];

   return {dp.back(), nullptr};
}

/// DPLoop
filter_t NOfEx_evaluateImpl1_exact(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<Roaring*> dp(self->n + 1);
   /// Copy bm of first child if immutable, otherwise use it directly
   auto tmp = self->children[0]->evaluate(db, dbp);
   if (tmp.mutable_res) {
      /// Do not need to delete tmp.mutable_res later, because dp[0] will be deleted
      dp[0] = tmp.mutable_res;
   } else {
      dp[0] = new Roaring(*tmp.immutable_res);
   }
   /// Initialize all bitmaps. Delete them later.
   for (unsigned i = 1; i < self->n + 1; ++i)
      dp[i] = new Roaring();

   for (unsigned i = 1; i < self->children.size(); ++i) {
      auto bm = self->children[i]->evaluate(db, dbp);
      /// positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the conjunction would return 0
      for (unsigned j = std::min(self->n, i); j >= 1; --j) {
         *dp[j] |= *dp[j - 1] & *bm.getAsConst();
      }
      *dp[0] |= *bm.getAsConst();
      bm.free();
   }

   /// Delete
   for (unsigned i = 0; i < self->n - 1; ++i)
      delete dp[i];

   /// Because exact, we remove all that have too many
   *dp[self->n - 1] -= *dp[self->n];

   delete dp[self->n];

   return {dp[self->n - 1], nullptr};
}

// N-Way Heap-Merge, for threshold queries
filter_t NOfEx_evaluateImpl2_threshold(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<filter_t> child_maps;
   struct bitmap_iterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<bitmap_iterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(db, dbp);
      child_maps.push_back(tmp);
      /// Invariant: All heap members 'cur' field must contain an element that needs to be processed
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end())
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
   }

   /// stl heap is max-heap. We want min, therefore we define a greater-than sorter
   /// as opposed to the standard less-than sorter
   auto min_heap_sort = [](const bitmap_iterator& a,
                           const bitmap_iterator& b) {
      return *a.cur > *b.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);

   auto ret = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = -1;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);
      /// Take element and ensure invariant
      uint32_t val = *iterator_heap.back().cur;
      iterator_heap.back().cur++;
      if (iterator_heap.back().cur == iterator_heap.back().end) {
         iterator_heap.pop_back();
      } else {
         std::push_heap(iterator_heap.begin(), iterator_heap.end(), min_heap_sort);
      }
      if (val == last_val) {
         cur_count++;
      } else {
         if (cur_count >= self->n) {
            buffer.push_back(last_val);
            if (buffer.size() == BUFFER_SIZE) {
               ret->addMany(BUFFER_SIZE, &buffer[0]);
               buffer.clear();
            }
         }
         last_val = val;
         cur_count = 1;
      }
   }
   if (cur_count >= self->n) {
      buffer.push_back(last_val);
   }

   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), &buffer[0]);
   }

   for (auto& child_map : child_maps)
      child_map.free();

   return {ret, nullptr};
}

// N-Way Heap-Merge, for exact queries
filter_t NOfEx_evaluateImpl2_exact(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<filter_t> child_maps;
   struct bitmap_iterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<bitmap_iterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(db, dbp);
      child_maps.push_back(tmp);
      /// Invariant: All heap members 'cur' field must contain an element that needs to be processed
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end())
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
   }

   /// stl heap is max-heap. We want min, therefore we define a greater-than sorter
   /// as opposed to the standard less-than sorter
   auto sorter = [](const bitmap_iterator& a,
                    const bitmap_iterator& b) {
      return *a.cur > *b.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), sorter);

   auto ret = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = UINT32_MAX;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      /// Take element and ensure invariant
      uint32_t val = *iterator_heap.back().cur;
      iterator_heap.back().cur++;
      if (iterator_heap.back().cur == iterator_heap.back().end) {
         iterator_heap.pop_back();
      } else {
         std::push_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      }
      if (val == last_val) {
         cur_count++;
      } else {
         if (cur_count == self->n) {
            buffer.push_back(last_val);
            if (buffer.size() == BUFFER_SIZE) {
               ret->addMany(BUFFER_SIZE, &buffer[0]);
               buffer.clear();
            }
         }
         last_val = val;
         cur_count = 1;
      }
   }
   if (cur_count == self->n) {
      buffer.push_back(last_val);
   }

   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), &buffer[0]);
   }

   for (auto& child_map : child_maps)
      child_map.free();

   return {ret, nullptr};
}

filter_t NOfEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   switch (impl) {
      case 0:
         if (exactly) {
            return NOfEx_evaluateImpl0_exact(this, db, dbp);
         } else {
            return NOfEx_evaluateImpl0_threshold(this, db, dbp);
         }
      case 1:
      default:
         if (exactly) {
            return NOfEx_evaluateImpl1_exact(this, db, dbp);
         } else {
            return NOfEx_evaluateImpl1_threshold(this, db, dbp);
         }
      case 2:
         if (exactly) {
            return NOfEx_evaluateImpl2_exact(this, db, dbp);
         } else {
            return NOfEx_evaluateImpl2_threshold(this, db, dbp);
         }
   }
}

filter_t NegEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   auto tmp = child->evaluate(db, dbp);
   auto ret = tmp.mutable_res ? tmp.mutable_res : new Roaring(*tmp.immutable_res);
   ret->flip(0, dbp.sequenceCount);
   return {ret, nullptr};
}

filter_t DateBetwEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   if (open_from && open_to) {
      auto ret = new Roaring();
      ret->addRange(0, dbp.sequenceCount);
      return {ret, nullptr};
   }

   auto ret = new Roaring;
   auto base = &dbp.meta_store.sid_to_date[0];
   for (const chunk_t& chunk : dbp.get_chunks()) {
      auto begin = &dbp.meta_store.sid_to_date[chunk.offset];
      auto end = &dbp.meta_store.sid_to_date[chunk.offset + chunk.count];
      uint32_t lower = open_from ? begin - base : std::lower_bound(begin, end, this->from) - base;
      uint32_t upper = open_to ? end - base : std::upper_bound(begin, end, this->to) - base;
      ret->addRange(lower, upper);
   }
   return {ret, nullptr};
}

filter_t DateBetwEx::select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   if (open_from && open_to) {
      return in_filter;
   } else {
      Roaring* ret;
      if (in_filter.mutable_res) {
         ret = in_filter.mutable_res;
      } else {
         ret = new Roaring(*in_filter.getAsConst());
      }
      auto base = &dbp.meta_store.sid_to_date[0];
      uint32_t lower = 0;
      uint32_t upper = 0;
      for (const chunk_t& chunk : dbp.get_chunks()) {
         auto begin = &dbp.meta_store.sid_to_date[chunk.offset];
         auto end = &dbp.meta_store.sid_to_date[chunk.offset + chunk.count];
         lower = open_from ? begin - base : std::lower_bound(begin, end, this->from) - base;
         ret->removeRange(upper, lower);
         upper = open_to ? end - base : std::upper_bound(begin, end, this->to) - base;
      }
      if (!open_to) {
         ret->removeRange(upper, dbp.sequenceCount);
      }
      return {ret, nullptr};
   }
}

filter_t DateBetwEx::neg_select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   if (open_from && open_to) {
      return in_filter;
   } else {
      Roaring* ret;
      if (in_filter.mutable_res) {
         ret = in_filter.mutable_res;
      } else {
         ret = new Roaring(*in_filter.getAsConst());
      }
      auto base = &dbp.meta_store.sid_to_date[0];
      for (const chunk_t& chunk : dbp.get_chunks()) {
         auto begin = &dbp.meta_store.sid_to_date[chunk.offset];
         auto end = &dbp.meta_store.sid_to_date[chunk.offset + chunk.count];
         uint32_t lower = open_from ? begin - base : std::lower_bound(begin, end, this->from) - base;
         uint32_t upper = open_to ? end - base : std::upper_bound(begin, end, this->to) - base;
         ret->removeRange(lower, upper);
      }
      return {ret, nullptr};
   }
}

filter_t NucEqEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   return {nullptr, dbp.seq_store.bm(position, value)};
}

filter_t NucMbEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   if (!negated) {
      /// Normal case
      return {dbp.seq_store.bma(position, value), nullptr};
   } else {
      /// The bitmap of this->value has been flipped... still have to union it with the other symbols
      return {dbp.seq_store.bma_neg(position, value), nullptr};
   }
}

filter_t PangoLineageEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   if (lineageKey == UINT32_MAX) return {new Roaring(), nullptr};
   if (includeSubLineages) {
      return {nullptr, &dbp.meta_store.sublineage_bitmaps[lineageKey]};
   } else {
      return {nullptr, &dbp.meta_store.lineage_bitmaps[lineageKey]};
   }
}

filter_t CountryEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   return {nullptr, &dbp.meta_store.country_bitmaps[countryKey]};
}

filter_t RegionEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   return {nullptr, &dbp.meta_store.region_bitmaps[regionKey]};
}

filter_t PosNEqEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq = 0; seq < dbp.sequenceCount; seq++) {
      if (dbp.seq_store.N_bitmaps[seq].contains(position)) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   return {ret, nullptr};
}

filter_t PosNEqEx::select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq : *in_filter.getAsConst()) {
      if (dbp.seq_store.N_bitmaps[seq].contains(position)) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {ret, nullptr};
}

filter_t PosNEqEx::neg_select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq : *in_filter.getAsConst()) {
      if (!dbp.seq_store.N_bitmaps[seq].contains(position)) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {ret, nullptr};
}

filter_t StrEqEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq = 0; seq < dbp.sequenceCount; seq++) {
      if (dbp.meta_store.cols[column][seq] == value) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   return {ret, nullptr};
}

filter_t StrEqEx::select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq : *in_filter.getAsConst()) {
      if (dbp.meta_store.cols[column][seq] == value) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {ret, nullptr};
}

filter_t StrEqEx::neg_select(const Database& /*db*/, const DatabasePartition& dbp, filter_t in_filter) {
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq : *in_filter.getAsConst()) {
      if (dbp.meta_store.cols[column][seq] != value) {
         buffer.push_back(seq);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, buffer.data());
            buffer.clear();
         }
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
   in_filter.free();
   return {ret, nullptr};
}

filter_t FullEx::evaluate(const Database&, const DatabasePartition& dbp) {
   Roaring* ret = new Roaring();
   ret->addRange(0, dbp.sequenceCount);
   return {ret, nullptr};
}

filter_t EmptyEx::evaluate(const Database&, const DatabasePartition&) {
   return {new Roaring(), nullptr};
}
} // namespace silo;

std::vector<silo::filter_t> silo::execute_predicate(const silo::Database& db, const BoolExpression* filter) {
   std::vector<silo::filter_t> partition_filters(db.partitions.size());
   tbb::blocked_range<size_t> r(0, db.partitions.size(), 1);
   tbb::parallel_for(r.begin(), r.end(), [&](const size_t& i) {
      std::unique_ptr<BoolExpression> part_filter = filter->simplify(db, db.partitions[i]);
      partition_filters[i] = part_filter->evaluate(db, db.partitions[i]);
   });
   return partition_filters;
}

silo::QueryResult silo::execute_query(const silo::Database& db, const std::string& query, std::ostream& parse_out, std::ostream& perf_out) {
   rapidjson::Document doc;
   doc.Parse(query.c_str());
   if (!doc.HasMember("filter") || !doc["filter"].IsObject() ||
       !doc.HasMember("action") || !doc["action"].IsObject()) {
      throw QueryParseException("Query json must contain filter and action.");
   }

   std::vector<std::string> simplified_queries(db.partitions.size());

   QueryResult query_result;
   std::unique_ptr<BoolExpression> filter;
   {
      BlockTimer timer(query_result.parseTime);
      filter = parse_expression(db, doc["filter"], 0);
      parse_out << "Parsed query: " << filter->to_string(db) << std::endl;
   }

   perf_out << "Parse: " << std::to_string(query_result.parseTime) << " microseconds\n";

   std::vector<silo::filter_t> partition_filters(db.partitions.size());
   {
      BlockTimer timer(query_result.filterTime);
      tbb::blocked_range<size_t> r(0, db.partitions.size(), 1);
      tbb::parallel_for(r.begin(), r.end(), [&](const size_t& i) {
         std::unique_ptr<BoolExpression> part_filter = filter->simplify(db, db.partitions[i]);
         simplified_queries[i] = part_filter->to_string(db);
         partition_filters[i] = part_filter->evaluate(db, db.partitions[i]);
      });
   }
   for (unsigned i = 0; i < db.partitions.size(); ++i) {
      parse_out << "Simplified query for partition " << i << ": " << simplified_queries[i] << std::endl;
   }
   perf_out << "Execution (filter): " << std::to_string(query_result.filterTime) << " microseconds\n";

   {
      BlockTimer timer(query_result.actionTime);
      const auto& action = doc["action"];
      assert(action.HasMember("type"));
      assert(action["type"].IsString());
      const auto& action_type = action["type"].GetString();

      if (action.HasMember("groupByFields")) {
         assert(action["groupByFields"].IsArray());
         std::vector<std::string> groupByFields;
         for (const auto& it : action["groupByFields"].GetArray()) {
            groupByFields.emplace_back(it.GetString());
         }
         if (strcmp(action_type, "Aggregated") == 0) {
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
         } else {
            query_result.queryResult = response::ErrorResult{"Unknown action", std::string(action_type) + " is not a valid action"};
         }
      } else {
         if (strcmp(action_type, "Aggregated") == 0) {
            unsigned count = execute_count(db, partition_filters);
            query_result.queryResult = response::AggregationResult{count};
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
            double min_proportion = 0.02;
            if (action.HasMember("minProportion") && action["minProportion"].IsDouble()) {
               if (action["minProportion"].GetDouble() <= 0.0) {
                  query_result.queryResult = response::ErrorResult{"Invalid proportion", "minProportion must be in interval (0.0,1.0]"};
                  return query_result;
               }
               min_proportion = action["minProportion"].GetDouble();
            }
            std::vector<MutationProportion> mutations = execute_mutations(db, partition_filters, min_proportion, perf_out);

            std::vector<response::MutationProportion> output_mutation_proportions(mutations.size());
            std::transform(
               mutations.begin(),
               mutations.end(),
               output_mutation_proportions.begin(),
               [](MutationProportion mutation_proportion) {
                  return response::MutationProportion{
                     mutation_proportion.mut_from + std::to_string(mutation_proportion.position) + mutation_proportion.mut_to,
                     mutation_proportion.proportion,
                     mutation_proportion.count};
               });
            query_result.queryResult = output_mutation_proportions;
         } else {
            query_result.queryResult = response::ErrorResult{"Unknown action", std::string(action_type) + " is not a valid action"};
         }
      }
   }

   perf_out << "Execution (action): " << std::to_string(query_result.actionTime) << " microseconds\n";

   return query_result;
}
