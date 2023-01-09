

#include "silo/query_engine.h"
#include "rapidjson/document.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_for_each.h"
#include <silo/common/PerfEvent.hpp>
#include <syncstream>

namespace silo {

using roaring::Roaring;

std::unique_ptr<BoolExpression> to_ex(const Database& db, const rapidjson::Value& js) {
   assert(js.HasMember("type"));
   assert(js["type"].IsString());
   std::string type = js["type"].GetString();
   if (type == "And") {
      auto ret = std::make_unique<AndEx>();
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return to_ex(db, js); });
      return ret;
   } else if (type == "Or") {
      auto ret = std::make_unique<OrEx>();
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return to_ex(db, js); });
      return ret;
   } else if (type == "N-Of") {
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      assert(js.HasMember("n"));
      assert(js["n"].IsUint());

      auto ret = std::make_unique<NOfEx>(js["n"].GetUint(), 0, js["exactly"].GetBool());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(ret->children), [&](const rapidjson::Value& js) { return to_ex(db, js); });
      if (js.HasMember("impl") && js["impl"].IsUint()) {
         ret->impl = js["impl"].GetUint();
      }
      return ret;
   } else if (type == "Neg") {
      auto ret = std::make_unique<NegEx>();
      ret->child = to_ex(db, js["child"]);
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
      auto ret = std::make_unique<NucEqEx>();
      ret->position = js["position"].GetUint();
      const std::string& s = js["value"].GetString();
      if (s.at(0) == '.') {
         char c = db.global_reference[0].at(ret->position);
         ret->value = to_symbol(c);
      } else {
         ret->value = to_symbol(s.at(0));
      }
      return ret;
   } else if (type == "NucMut") {
      assert(js.HasMember("position"));
      unsigned pos = js["position"].GetUint();
      char ref_symbol = db.global_reference[0].at(pos);
      return std::make_unique<NegEx>(std::make_unique<NucEqEx>(pos, silo::to_symbol(ref_symbol)));
   } else if (type == "NucMbEq") {
      auto ret = std::make_unique<NucMbEx>();
      ret->position = js["position"].GetUint();
      const std::string& s = js["value"].GetString();
      if (s.at(0) == '.') {
         char c = db.global_reference[0].at(ret->position);
         ret->value = to_symbol(c);
      } else {
         ret->value = to_symbol(s.at(0));
      }
      return ret;
   } else if (type == "PangoLineage") {
      bool includeSubLineages = js["includeSubLineages"].GetBool();
      std::string lineage = js["value"].GetString();
      std::transform(lineage.begin(), lineage.end(), lineage.begin(), ::toupper);
      lineage = resolve_alias(db.alias_key, lineage);
      uint32_t lineageKey = db.dict->get_pangoid(lineage);
      return std::make_unique<PangoLineageEx>(lineageKey, includeSubLineages);
   } else if (type == "StrEq") {
      const std::string& col = js["column"].GetString();
      if (col == "country") {
         return std::make_unique<CountryEx>(db.dict->get_countryid(js["value"].GetString()));
      } else if (col == "region") {
         return std::make_unique<RegionEx>(db.dict->get_regionid(js["value"].GetString()));
      } else {
         return std::make_unique<StrEqEx>(js["column"].GetString(), js["value"].GetString());
      }
   } else {
      throw QueryParseException("Unknown object type");
   }
}

filter_t AndEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   auto tmp = children[0]->evaluate(db, dbp);
   Roaring* ret;
   if (tmp.mutable_res) {
      ret = tmp.mutable_res;
      *ret &= *children[1]->evaluate(db, dbp).getAsConst();
   } else {
      auto x = roaring_bitmap_and(&tmp.immutable_res->roaring, &children[1]->evaluate(db, dbp).getAsConst()->roaring);
      ret = new Roaring(x);
   }
   for (unsigned i = 2; i < children.size(); i++) {
      auto& child = children[i];
      auto bm = child->evaluate(db, dbp);
      *ret &= *bm.getAsConst();
   }
   return {ret, nullptr};
}

filter_t OrEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   unsigned n = children.size();
   const Roaring* union_tmp[n];
   filter_t child_res[n];
   for (unsigned i = 0; i < n; i++) {
      auto tmp = children[i]->evaluate(db, dbp);
      child_res[i] = tmp;
      union_tmp[i] = tmp.mutable_res ? tmp.mutable_res : tmp.immutable_res;
   }
   Roaring* ret = new Roaring(Roaring::fastunion(children.size(), union_tmp));
   for (unsigned i = 0; i < n; i++) {
      if (child_res[i].mutable_res) {
         delete child_res[i].mutable_res;
      }
   }
   return {ret, nullptr};
}

void vec_and_not(std::vector<uint32_t>& dest, const std::vector<uint32_t>& v1, const std::vector<uint32_t>& v2) {
   std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(dest));
}

filter_t NOfEx_evaluateImpl0(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   if (self->exactly) {
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
      std::vector<uint32_t> correct;
      vec_and_not(correct, at_least, too_much);
      return {new Roaring(correct.size(), &correct[0]), nullptr};
   } else {
      std::vector<uint16_t> count;
      std::vector<uint32_t> correct;
      count.resize(dbp.sequenceCount);
      for (auto& child : self->children) {
         auto bm = child->evaluate(db, dbp);
         for (uint32_t id : *bm.getAsConst()) {
            if (++count[id] == self->n) {
               correct.push_back(id);
            }
         }
         bm.free();
      }
      return {new Roaring(correct.size(), &correct[0]), nullptr};
   }
}

// DPLoop
filter_t NOfEx_evaluateImpl1(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
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

   for (unsigned i = 1; i < self->n; ++i) {
      auto bm = self->children[i]->evaluate(db, dbp);
      /// positions higher than (i-1) cannot have been reached yet, are therefore all 0s and the conjunction would return 0
      for (unsigned j = std::min(self->n - 1, i); j >= 1; ++j) {
         *dp[j] |= *dp[j - 1] & *bm.getAsConst();
      }
      *dp[0] |= *bm.getAsConst();
      if (self->exactly && i >= self->n) {
         roaring::api::roaring_bitmap_andnot_inplace(&dp[self->n - 1]->roaring, &bm.getAsConst()->roaring);
      }
      bm.free();
   }

   /// Delete
   for (unsigned i = 0; i < self->n - 1; ++i)
      delete dp[i];

   return {dp.back(), nullptr};
}

// N-Way Heap-Merge, for threshold queries
filter_t NOfEx_evaluateImpl2threshold(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<filter_t> child_maps;
   struct bitmap_iterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<bitmap_iterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(db, dbp);
      child_maps.push_back(tmp);
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end())
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
   }

   auto sorter = [](const bitmap_iterator& a,
                    const bitmap_iterator& b) {
      return *a.cur > *b.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), sorter);

   auto ret = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = -1;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      uint32_t val = *iterator_heap.back().cur;
      cur_count = val == last_val ? cur_count + 1 : 1;
      if (cur_count == self->n) {
         buffer.push_back(val);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, &buffer[0]);
            buffer.clear();
         }
      } else {
         last_val = val;
         iterator_heap.back().cur++;
         if (iterator_heap.back().cur == iterator_heap.back().end) {
            iterator_heap.pop_back();
         } else {
            std::push_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
         }
      }
   }

   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), &buffer[0]);
   }

   for (auto& child_map : child_maps)
      child_map.free();

   return {ret, nullptr};
}

// N-Way Heap-Merge, for exact queries
filter_t NOfEx_evaluateImpl2exact(const NOfEx* self, const Database& db, const DatabasePartition& dbp) {
   std::vector<filter_t> child_maps;
   struct bitmap_iterator {
      roaring::RoaringSetBitForwardIterator cur;
      roaring::RoaringSetBitForwardIterator end;
   };
   std::vector<bitmap_iterator> iterator_heap;
   for (const auto& child : self->children) {
      auto tmp = child->evaluate(db, dbp);
      child_maps.push_back(tmp);
      if (tmp.getAsConst()->begin() != tmp.getAsConst()->end())
         iterator_heap.push_back({tmp.getAsConst()->begin(), tmp.getAsConst()->end()});
   }

   auto sorter = [](const bitmap_iterator& a,
                    const bitmap_iterator& b) {
      return *a.cur > *b.cur;
   };

   std::make_heap(iterator_heap.begin(), iterator_heap.end(), sorter);

   auto ret = new Roaring();

   constexpr size_t BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer;
   buffer.reserve(BUFFER_SIZE);

   uint32_t last_val = -1;
   uint32_t cur_count = 0;

   while (!iterator_heap.empty()) {
      std::pop_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
      uint32_t val = *iterator_heap.back().cur;
      if (cur_count == self->n && val != last_val) {
         cur_count = 1;
         buffer.push_back(val);
         if (buffer.size() == BUFFER_SIZE) {
            ret->addMany(BUFFER_SIZE, &buffer[0]);
            buffer.clear();
         }
      } else {
         cur_count = val == last_val ? cur_count + 1 : 1;
         last_val = val;
         iterator_heap.back().cur++;
         if (iterator_heap.back().cur == iterator_heap.back().end) {
            iterator_heap.pop_back();
         } else {
            std::push_heap(iterator_heap.begin(), iterator_heap.end(), sorter);
         }
      }
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
      default:
      case 1:
         return NOfEx_evaluateImpl1(this, db, dbp);
      case 0:
         return NOfEx_evaluateImpl0(this, db, dbp);
      case 2:
         if (exactly) {
            return NOfEx_evaluateImpl2exact(this, db, dbp);
         } else {
            return NOfEx_evaluateImpl2threshold(this, db, dbp);
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
      uint32_t lower = open_to ? begin - base : std::lower_bound(begin, end, this->from) - base;
      uint32_t upper = open_to ? end - base : std::upper_bound(begin, end, this->to) - base;
      ret->addRange(lower, upper);
   }
   return {ret, nullptr};
}

filter_t NucEqEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   return {nullptr, dbp.seq_store.bm(position, value)};
}

filter_t NucMbEx::evaluate(const Database& /*db*/, const DatabasePartition& dbp) {
   return {dbp.seq_store.bma(this->position, this->value), nullptr};
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

filter_t StrEqEx::evaluate(const Database& db, const DatabasePartition& dbp) {
   unsigned columnIndex = db.dict->get_colid(this->column);
   constexpr unsigned BUFFER_SIZE = 1024;
   std::vector<uint32_t> buffer(BUFFER_SIZE);
   Roaring* ret = new Roaring();
   for (uint32_t seq : dbp.meta_store.cols[columnIndex]) {
      buffer.push_back(seq);
      if (buffer.size() == BUFFER_SIZE) {
         ret->addMany(BUFFER_SIZE, buffer.data());
         buffer.clear();
      }
   }
   if (buffer.size() > 0) {
      ret->addMany(buffer.size(), buffer.data());
   }
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

silo::result_s silo::execute_query(const silo::Database& db, const std::string& query, std::ostream& res_out, std::ostream& perf_out) {
   std::cout << "Executing query: " << query << std::endl;

   rapidjson::Document doc;
   doc.Parse(query.c_str());
   if (!doc.HasMember("filter") || !doc["filter"].IsObject() ||
       !doc.HasMember("action") || !doc["action"].IsObject()) {
      throw QueryParseException("Query json must contain filter and action.");
   }

   result_s ret;
   std::unique_ptr<BoolExpression> filter;
   {
      BlockTimer timer(ret.parse_time);
      filter = to_ex(db, doc["filter"]);
      std::cout << "Parsed query: " << filter->to_string(db) << std::endl;
   }

   perf_out << "Parse: " << std::to_string(ret.parse_time) << " microseconds\n";

   std::vector<silo::filter_t> partition_filters(db.partitions.size());
   {
      BlockTimer timer(ret.filter_time);
      tbb::blocked_range<size_t> r(0, db.partitions.size(), 1);
      tbb::parallel_for(r.begin(), r.end(), [&](const size_t& i) {
         std::unique_ptr<BoolExpression> part_filter = filter->simplify(db, db.partitions[i]);
         std::osyncstream(std::cout) << "Simplified query: " << part_filter->to_string(db) << std::endl;
         partition_filters[i] = part_filter->evaluate(db, db.partitions[i]);
      });
   }
   perf_out << "Execution (filter): " << std::to_string(ret.filter_time) << " microseconds\n";

   {
      BlockTimer timer(ret.action_time);
      const auto& action = doc["action"];
      assert(action.HasMember("type"));
      assert(action["type"].IsString());
      const auto& action_type = action["type"].GetString();

      if (action.HasMember("groupByFields")) {
         assert(action["groupByFields"].IsArray());
         std::vector<std::string> groupByFields;
         for (const auto& it : action["groupByFields"].GetArray()) {
            groupByFields.push_back(it.GetString());
         }
         if (strcmp(action_type, "Aggregated") == 0) {
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
         } else {
            ret.return_message = "Unknown action ";
            ret.return_message += action_type;
         }
      } else {
         if (strcmp(action_type, "Aggregated") == 0) {
            unsigned count = execute_count(db, partition_filters);
            ret.return_message = "{\"count\": " + std::to_string(count) + "}";
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
            double min_proportion = 0.02;
            if (action.HasMember("minProportion") && action["minProportion"].IsDouble()) {
               min_proportion = action["minProportion"].GetDouble();
            }
            std::vector<mutation_proportion> mutations = execute_mutations(db, partition_filters, min_proportion);
            ret.return_message = "";
            for (auto& s : mutations) {
               ret.return_message += "{\"mutation\":\"";
               ret.return_message += s.mut_from;
               ret.return_message += std::to_string(s.position);
               ret.return_message += s.mut_to;
               ret.return_message += "\",\"proportion\":" + std::to_string(s.proportion) +
                  ",\"count\":" + std::to_string(s.count) + "},";
            }
         } else {
            ret.return_message = "Unknown action ";
            ret.return_message += action_type;
         }
      }
   }

   perf_out << "Execution (action): " << std::to_string(ret.action_time) << " microseconds\n";

   res_out << ret.return_message;

   return ret;
}
