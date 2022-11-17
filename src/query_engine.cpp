

#include "silo/query_engine.h"

namespace silo {

struct QueryParseException : public std::exception {
   private:
   const char* message;

   public:
   explicit QueryParseException(const std::string& msg) : message(msg.c_str()) {}

   [[nodiscard]] const char* what() const noexcept override {
      return message;
   }
};

struct Expression {
   // For future, maybe different (return) types of expressions?
   // TypeV type;

   /// Constructor
   explicit Expression(const rapidjson::Value& js) {}

   /// Destructor
   virtual ~Expression() = default;

   /// Evaluate the expression by interpreting it.
   virtual Roaring* evaluate(const SequenceStore&, const MetaStore&) {
      throw std::runtime_error("Not Implemented exception. Does not override Expression::evaluate.");
   };

   /* Maybe generate code in the future
      /// Build the expression LLVM IR code.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual llvm::Value *build(llvm::IRBuilder<> &builder, llvm::Value *args);*/
};

std::unique_ptr<Expression> to_ex(const rapidjson::Value& js);

struct VectorEx : public Expression {
   std::vector<std::unique_ptr<Expression>> children;

   explicit VectorEx(const rapidjson::Value& js) : Expression(js) {
      assert(js.HasMember("children"));
      assert(js["children"].IsArray());
      std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                     std::back_inserter(children), to_ex);
   }
};

struct AndEx : public VectorEx {
   explicit AndEx(const rapidjson::Value& js) : VectorEx(js) {}

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct OrEx : public VectorEx {
   explicit OrEx(const rapidjson::Value& js) : VectorEx(js) {}

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct NOfEx : public VectorEx {
   unsigned n;
   unsigned impl;
   bool exactly;

   explicit NOfEx(const rapidjson::Value& js) : VectorEx(js) {
      n = js["n"].GetUint();
      exactly = js["exactly"].GetBool();
      if (js.HasMember("impl")) {
         impl = js["impl"].GetUint();
      } else {
         impl = 0;
      }
   }

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct NegEx : public Expression {
   std::unique_ptr<Expression> child;

   explicit NegEx(const rapidjson::Value& js) : Expression(js) {
      child = to_ex(js["child"]);
   }

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct DateBetwEx : public Expression {
   time_t from;
   bool open_from;
   time_t to;
   bool open_to;

   explicit DateBetwEx(const rapidjson::Value& js) : Expression(js) {
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

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct NucEqEx : public Expression {
   unsigned position;
   Symbol value;

   explicit NucEqEx(const rapidjson::Value& js) : Expression(js) {
      position = js["position"].GetUint();
      value = to_symbol(js["value"].GetString()[0]);
   }

   Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb) override;
};

struct NucMbEx : public Expression {
   unsigned position;
   Symbol value;

   explicit NucMbEx(const rapidjson::Value& js) : Expression(js) {
      position = js["position"].GetUint();
      value = to_symbol(js["value"].GetString()[0]);
   }
};

struct NucMutEx : public Expression {
   unsigned position;

   explicit NucMutEx(const rapidjson::Value& js) : Expression(js) {
      position = js["position"].GetUint();
   }
};

struct PangoLineageEx : public Expression {
   std::string value;
   bool includeSubLineages;

   explicit PangoLineageEx(const rapidjson::Value& js) : Expression(js) {
      includeSubLineages = js["includeSubLineages"].GetBool();
      value = js["value"].GetString();
   }
};

struct StrEqEx : public Expression {
   std::string column;
   std::string value;

   explicit StrEqEx(const rapidjson::Value& js) : Expression(js) {
      column = js["column"].GetString();
      value = js["value"].GetString();
   }
};

std::unique_ptr<Expression> to_ex(const rapidjson::Value& js) {
   std::string type = js["type"].GetString();
   if (type == "And") {
      return std::make_unique<AndEx>(js);
   } else if (type == "Or") {
      return std::make_unique<OrEx>(js);
   } else if (type == "N-Of") {
      return std::make_unique<NOfEx>(js);
   } else if (type == "Neg") {
      return std::make_unique<NegEx>(js);
   } else if (type == "DateBetw") {
      return std::make_unique<DateBetwEx>(js);
   } else if (type == "NucEq") {
      return std::make_unique<NucEqEx>(js);
   } else if (type == "NucMut") {
      return std::make_unique<NucMutEx>(js);
   } else if (type == "PangoLineage") {
      return std::make_unique<PangoLineageEx>(js);
   } else if (type == "StrEq") {
      return std::make_unique<StrEqEx>(js);
   } else {
      throw QueryParseException("Unknown object type");
   }
}

Roaring* AndEx::evaluate(const silo::SequenceStore& db, const silo::MetaStore& mdb) {
   auto ret = children[0]->evaluate(db, mdb);
   for (auto& child : children) {
      auto bm = child->evaluate(db, mdb);
      *ret &= *bm;
      delete bm;
   }
   return ret;
}

Roaring* OrEx::evaluate(const silo::SequenceStore& db, const silo::MetaStore& mdb) {
   unsigned n = children.size();
   const Roaring* child_res[n];
   for (unsigned i = 0; i < n; i++) {
      child_res[i] = children[i]->evaluate(db, mdb);
   }
   auto ret = new Roaring(Roaring::fastunion(children.size(), child_res));
   for (unsigned i = 0; i < n; i++) {
      delete child_res[i];
   }
   return ret;
}

void vec_and_not(std::vector<uint32_t> dest, std::vector<uint32_t> v1, std::vector<uint32_t> v2) {
   // And not of correct and incorrect
   auto it1 = v1.begin(), it2 = v2.begin(),
        limit1 = v1.end(), limit2 = v2.end();
   while (true) {
      if (it1 >= limit1) return;
      if (it2 >= limit2) {
         dest.insert(dest.end(), it1, limit1);
         return;
      }
      if (*it1 == *it2) {
         ++it1;
         ++it2;
      } else if (*it1 < *it2) {
         dest.emplace_back(*it1);
      } else { // *it1 > *it2
         ++it1;
      }
   }
}

Roaring* NOfExevaluateImpl0(const NOfEx* self, const silo::SequenceStore& db, const silo::MetaStore& mdb) {
   if (self->exactly) {
      std::vector<uint16_t> count;
      std::vector<uint32_t> at_least;
      std::vector<uint32_t> too_much;
      count.resize(db.sequenceCount);
      for (auto& child : self->children) {
         auto bm = child->evaluate(db, mdb);
         for (uint32_t id : *bm) {
            if (++count[id] == self->n) {
               at_least.push_back(id);
            } else if (count[id] == self->n + 1) {
               too_much.push_back(id);
            }
         }
         delete bm;
      }
      std::vector<uint32_t> correct;
      vec_and_not(correct, at_least, too_much);
      return new Roaring(correct.size(), &correct[0]);

   } else {
      std::vector<uint16_t> count;
      std::vector<uint32_t> correct;
      count.resize(db.sequenceCount);
      for (auto& child : self->children) {
         auto bm = child->evaluate(db, mdb);
         for (uint32_t id : *bm) {
            if (++count[id] == self->n) {
               correct.push_back(id);
            }
         }
         delete bm;
      }
      return new Roaring(correct.size(), &correct[0]);
   }
}

Roaring* NOfEx::evaluate(const silo::SequenceStore& db, const silo::MetaStore& mdb) {
   switch (impl) {
      case 0:
      default:
         return NOfExevaluateImpl0(this, db, mdb);
   }
}

Roaring* NegEx::evaluate(const SequenceStore& db, const MetaStore& mdb) {
   auto ret = child->evaluate(db, mdb);
   ret->flip(0, db.sequenceCount);
   return ret;
}

Roaring* DateBetwEx::evaluate(const SequenceStore& db, const MetaStore& mdb) {
   if (open_from && open_to) {
      auto ret = new Roaring;
      ret->addRange(0, db.sequenceCount);
      return ret;
   }

   auto ret = new Roaring;
   auto base = &mdb.sid_to_date[0];
   for (const chunk_t& chunk : mdb.chunks) {
      auto begin = &mdb.sid_to_date[chunk.offset];
      auto end = &mdb.sid_to_date[chunk.offset + chunk.count];
      uint32_t lower = open_to ? begin - base : std::lower_bound(begin, end, this->from) - base;
      uint32_t upper = open_to ? end - base : std::upper_bound(begin, end, this->to) - base;
      ret->addRange(lower, upper);
   }
   return ret;
}

Roaring* NucEqEx::evaluate(const SequenceStore& db, const MetaStore&) {
   return new Roaring(*db.bm(position, to_symbol(value)));
}

} // namespace silo;

std::string silo::execute_query(const SequenceStore& db, const MetaStore& mdb, const std::string& query) {
   rapidjson::Document doc;
   doc.Parse(query.c_str());
   if (!doc.HasMember("filter") || !doc["filter"].IsObject() ||
       !doc.HasMember("action") || !doc["action"].IsObject()) {
      throw QueryParseException("Query json must contain filter and action.");
   }

   std::unique_ptr<Expression> filter = to_ex(doc["filter"]);
   // std::string action = doc["action"];
   Roaring* result = filter->evaluate(db, mdb);
   std::stringstream ret;
   ret << "{\"count\":" << result->cardinality() << "}";
   delete result;
   return ret.str();
}
