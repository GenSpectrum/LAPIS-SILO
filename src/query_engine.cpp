

#include "silo/query_engine.h"
#include "rapidjson/document.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_for_each.h"
#include <silo/common/PerfEvent.hpp>

namespace silo {

using roaring::Roaring;

enum ExType {
   AND,
   OR,
   NOF,
   NEG,
   INDEX_FILTER,
   PRED,
   EMPTY,
   FULL
};

struct BoolExpression {
   // For future, maybe different (return) types of expressions?
   // TypeV type;

   /// Constructor
   explicit BoolExpression(const rapidjson::Value& /*js*/) {}

   /// Constructor
   explicit BoolExpression() {}

   /// Destructor
   virtual ~BoolExpression() = default;

   virtual ExType type() const = 0;

   /// Evaluate the expression by interpreting it.
   /// If mutable bitmap is returned, caller must free the result
   virtual filter_t evaluate(const Database& /*db*/, const DatabasePartition& /*dbp*/) = 0;

   /// Transforms the expression to a human readable string.
   virtual std::string to_string(const Database& db) = 0;

   virtual std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const = 0;

   /* Maybe generate code in the future
      /// Build the expression LLVM IR code.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual llvm::Value *build(llvm::IRBuilder<> &builder, llvm::Value *args);*/
};

std::unique_ptr<BoolExpression> to_ex(const Database& db, const rapidjson::Value& js);

struct EmptyEx : public BoolExpression {
   ExType type() const override {
      return ExType::EMPTY;
   };

   /// EmptyEx should be simplified away.
   filter_t evaluate(const Database& /*db*/, const DatabasePartition& /*dbp*/) override {
      return {new Roaring(), nullptr};
   }

   std::string to_string(const Database& /*db*/) override {
      return "FALSE";
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<silo::EmptyEx>();
   }
};

struct FullEx : public BoolExpression {
   ExType type() const override {
      return ExType::FULL;
   };

   /// EmptyEx should be simplified away.
   filter_t evaluate(const Database& /*db*/, const DatabasePartition& dbp) override {
      Roaring* ret = new Roaring();
      ret->addRange(0, dbp.sequenceCount);
      return {ret, nullptr};
   }

   std::string to_string(const Database& /*db*/) override {
      return "TRUE";
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<silo::FullEx>();
   }
};

struct AndEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;

   explicit AndEx() {}

   ExType type() const override {
      return ExType::AND;
   };

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "(";
      for (auto& child : children) {
         res += child->to_string(db);
         res += " & ";
      }
      res += ")";
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp) const override {
      std::vector<std::unique_ptr<BoolExpression>> new_children;
      std::transform(children.begin(), children.end(),
                     std::back_inserter(new_children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
      std::unique_ptr<AndEx> ret = std::make_unique<AndEx>();
      for (unsigned i = 0; i < new_children.size(); i++) {
         auto& child = new_children[i];
         if (child->type() == FULL) {
            continue;
         } else if (child->type() == EMPTY) {
            return std::make_unique<EmptyEx>();
         } else if (child->type() == AND) {
            AndEx* or_child = dynamic_cast<AndEx*>(child.get());
            std::transform(or_child->children.begin(), or_child->children.end(),
                           std::back_inserter(new_children), [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); });
         } else {
            ret->children.push_back(std::move(child));
         }
      }
      if (ret->children.empty()) {
         return std::make_unique<EmptyEx>();
      }
      if (ret->children.size() == 1) {
         return std::move(ret->children[0]);
      }
      return ret;
   }
};

struct OrEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   explicit OrEx(const Database& db, const rapidjson::Value& js) {
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(children), [&](const rapidjson::Value& js) { return to_ex(db, js); });
   }

   explicit OrEx() {
   }

   ExType type() const override {
      return ExType::OR;
   };

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "(";
      for (auto& child : children) {
         res += child->to_string(db);
         res += " | ";
      }
      res += ")";
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp) const override {
      std::vector<std::unique_ptr<BoolExpression>> new_children;
      std::transform(children.begin(), children.end(),
                     std::back_inserter(new_children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
      std::unique_ptr<OrEx> ret = std::make_unique<OrEx>();
      for (unsigned i = 0; i < new_children.size(); i++) {
         auto& child = new_children[i];
         if (child->type() == EMPTY) {
            continue;
         } else if (child->type() == FULL) {
            return std::make_unique<FullEx>();
         } else if (child->type() == OR) {
            OrEx* or_child = dynamic_cast<OrEx*>(child.get());
            std::transform(or_child->children.begin(), or_child->children.end(),
                           std::back_inserter(new_children), [&](std::unique_ptr<BoolExpression>& c) { return std::move(c); });
         } else {
            ret->children.push_back(std::move(child));
         }
      }
      if (ret->children.empty()) {
         return std::make_unique<EmptyEx>();
      }
      if (ret->children.size() == 1) {
         return std::move(ret->children[0]);
      }
      return ret;
   }
};

struct NOfEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   unsigned n;
   unsigned impl;
   bool exactly;

   ExType type() const override {
      return ExType::NOF;
   };

   explicit NOfEx(unsigned n, unsigned impl, bool exactly) : n(n), impl(impl), exactly(exactly) {}

   explicit NOfEx(const Database& db, const rapidjson::Value& js) {
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(children), [&](const rapidjson::Value& js) { return to_ex(db, js); });
      n = js["n"].GetUint();
      exactly = js["exactly"].GetBool();
      if (js.HasMember("impl")) {
         impl = js["impl"].GetUint();
      } else {
         impl = 0;
      }
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res;
      if (exactly) {
         res = "[exactly-" + std::to_string(n) + "-of:";
      } else {
         res = "[" + std::to_string(n) + "-of:";
      }
      for (auto& child : children) {
         res += child->to_string(db);
         res += ", ";
      }
      res += "]";
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp) const override {
      std::unique_ptr<NOfEx> ret = std::make_unique<NOfEx>(n, impl, exactly);
      std::transform(children.begin(), children.end(),
                     std::back_inserter(ret->children), [&](const std::unique_ptr<BoolExpression>& c) { return c->simplify(db, dbp); });
      return ret;
   }
};

struct NegEx : public BoolExpression {
   std::unique_ptr<BoolExpression> child;

   ExType type() const override {
      return ExType::NEG;
   };

   explicit NegEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      child = to_ex(db, js["child"]);
   }

   explicit NegEx(std::unique_ptr<BoolExpression> child) : child(std::move(child)) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "!" + child->to_string(db);
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp) const override {
      std::unique_ptr<NegEx> ret = std::make_unique<NegEx>(child->simplify(db, dbp));
      if (ret->child->type() == ExType::NEG) {
         return std::move(dynamic_cast<NegEx*>(ret->child.get())->child);
      }
      return ret;
   }
};

struct DateBetwEx : public BoolExpression {
   time_t from;
   bool open_from;
   time_t to;
   bool open_to;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit DateBetwEx(time_t from, bool open_from, time_t to, bool open_to)
      : from(from), open_from(open_from), to(to), open_to(open_to) {}

   explicit DateBetwEx(const Database& /*db*/, const rapidjson::Value& js) : BoolExpression(js) {
      if (js["from"].IsNull()) {
         open_from = true;
      } else {
         open_from = false;

         struct std::tm tm {};
         std::istringstream ss(js["from"].GetString());
         ss >> std::get_time(&tm, "%Y-%m-%d");
         from = mktime(&tm);
      }

      if (js["to"].IsNull()) {
         open_to = true;
      } else {
         open_to = false;

         struct std::tm tm {};
         std::istringstream ss(js["to"].GetString());
         ss >> std::get_time(&tm, "%Y-%m-%d");
         to = mktime(&tm);
      }
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = "[Date-between ";
      res += (open_from ? "unbound" : std::to_string(from));
      res += " and ";
      res += (open_to ? "unbound" : std::to_string(to));
      res += "]";
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<DateBetwEx>(from, open_from, to, open_to);
   }
};

struct NucEqEx : public BoolExpression {
   unsigned position;
   Symbol value;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit NucEqEx(unsigned position, Symbol value) : position(position), value(value) {}

   explicit NucEqEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      position = js["position"].GetUint();
      const std::string& s = js["value"].GetString();
      if (s.at(0) == '.') {
         char c = db.global_reference[0].at(position);
         value = to_symbol(c);
      } else {
         value = to_symbol(s.at(0));
      }
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = std::to_string(position) + symbol_rep[value];
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp) const override {
      std::unique_ptr<BoolExpression> ret = std::make_unique<NucEqEx>(position, value);
      if (dbp.seq_store.positions[position].flipped_bitmap == value) { /// Bitmap of position is flipped! Introduce Neg
         return std::make_unique<NegEx>(std::move(ret));
      } else {
         return ret;
      }
   }
};

struct NucMbEx : public BoolExpression {
   unsigned position;
   Symbol value;
   bool negated = false;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit NucMbEx(unsigned position, Symbol value) : position(position), value(value) {}

   explicit NucMbEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      position = js["position"].GetUint();
      const std::string& s = js["value"].GetString();
      if (s.at(0) == '.') {
         char c = db.global_reference[0].at(position);
         value = to_symbol(c);
      } else {
         value = to_symbol(s.at(0));
      }
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = "?" + std::to_string(position) + symbol_rep[value];
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp) const override {
      std::unique_ptr<NucMbEx> ret = std::make_unique<NucMbEx>(position, value);
      if (dbp.seq_store.positions[position].flipped_bitmap == value) { /// Bitmap of reference is flipped! Introduce Neg
         ret->negated = true;
         return std::make_unique<NegEx>(std::move(ret));
      } else {
         return ret;
      }
   }
};

struct PangoLineageEx : public BoolExpression {
   uint32_t lineageKey;
   bool includeSubLineages;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit PangoLineageEx(uint32_t lineageKey, bool includeSubLineages)
      : lineageKey(lineageKey), includeSubLineages(includeSubLineages) {}

   explicit PangoLineageEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      includeSubLineages = js["includeSubLineages"].GetBool();
      std::string lineage = js["value"].GetString();
      std::transform(lineage.begin(), lineage.end(), lineage.begin(), ::toupper);
      lineage = resolve_alias(db.alias_key, lineage);
      lineageKey = db.dict->get_pangoid(lineage);
      std::cout << "Lineage: " << lineage << ": " << lineageKey << std::endl;
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = db.dict->get_pango(lineageKey);
      if (includeSubLineages) {
         res += ".*";
      }
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp) const override {
      if (lineageKey == UINT32_MAX) {
         return std::make_unique<EmptyEx>();
      }
      if (!this->includeSubLineages && !std::binary_search(dbp.sorted_lineages.begin(), dbp.sorted_lineages.end(), lineageKey)) {
         return std::make_unique<EmptyEx>();
      } else {
         return std::make_unique<PangoLineageEx>(lineageKey, includeSubLineages);
      }
   }
};

struct CountryEx : public BoolExpression {
   uint32_t countryKey;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit CountryEx(uint32_t countryKey) : countryKey(countryKey) {}

   explicit CountryEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      countryKey = db.dict->get_countryid(js["value"].GetString());
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "Country=" + db.dict->get_country(countryKey);
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<CountryEx>(countryKey);
   }
};

struct RegionEx : public BoolExpression {
   uint32_t regionKey;

   ExType type() const override {
      return ExType::INDEX_FILTER;
   };

   explicit RegionEx(uint32_t regionKey) : regionKey(regionKey) {
   }

   explicit RegionEx(const Database& db, const rapidjson::Value& js) : BoolExpression(js) {
      regionKey = db.dict->get_regionid(js["value"].GetString());
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "Region=" + db.dict->get_region(regionKey);
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<RegionEx>(regionKey);
   }
};

struct StrEqEx : public BoolExpression {
   std::string column;
   std::string value;

   ExType type() const override {
      return ExType::PRED;
   };

   explicit StrEqEx(const std::string& column, const std::string& value) : column(column), value(value) {}

   explicit StrEqEx(const Database& /*db*/, const rapidjson::Value& js) : BoolExpression(js) {
      column = js["column"].GetString();
      value = js["value"].GetString();
   }

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = column + "=" + value;
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<StrEqEx>(column, value);
   }
};

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
      return std::make_unique<OrEx>(db, js);
   } else if (type == "N-Of") {
      return std::make_unique<NOfEx>(db, js);
   } else if (type == "Neg") {
      return std::make_unique<NegEx>(db, js);
   } else if (type == "DateBetw") {
      return std::make_unique<DateBetwEx>(db, js);
   } else if (type == "NucEq") {
      return std::make_unique<NucEqEx>(db, js);
   } else if (type == "NucMut") {
      assert(js.HasMember("position"));
      unsigned pos = js["position"].GetUint();
      char ref_symbol = db.global_reference[0].at(pos);
      return std::make_unique<NegEx>(std::make_unique<NucEqEx>(pos, silo::to_symbol(ref_symbol)));
   } else if (type == "PangoLineage") {
      return std::make_unique<PangoLineageEx>(db, js);
   } else if (type == "StrEq") {
      const std::string& col = js["column"].GetString();
      if (col == "country") {
         return std::make_unique<CountryEx>(db, js);
      } else if (col == "region") {
         return std::make_unique<RegionEx>(db, js);
      } else {
         return std::make_unique<StrEqEx>(db, js);
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

} // namespace silo;

std::string execute_query_part(const silo::Database& db, const silo::DatabasePartition& dbp, const std::string& query) {
   using namespace silo;
   rapidjson::Document doc;
   doc.Parse(query.c_str());
   if (!doc.HasMember("filter") || !doc["filter"].IsObject() ||
       !doc.HasMember("action") || !doc["action"].IsObject()) {
      throw QueryParseException("Query json must contain filter and action.");
   }

   std::unique_ptr<BoolExpression> filter = to_ex(db, doc["filter"]);
   // std::string action = doc["action"];
   const Roaring* result = filter->evaluate(db, dbp).getAsConst();
   std::stringstream ret;
   ret << "{\"count\":" << result->cardinality() << "}";
   delete result;
   return ret.str();
}

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
         partition_filters[i] = filter->evaluate(db, db.partitions[i]);
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
            std::vector<mut_struct> mutations = execute_mutations(db, partition_filters, 0.02);
            ret.return_message = "";
            for (auto& s : mutations) {
               ret.return_message += "{\"mutation\":\"" + s.mutation +
                  "\",\"proportion\":" + std::to_string(s.proportion) +
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
