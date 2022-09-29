

#include "silo/query_engine.h"

namespace silo {


   struct Expression {
      // For future, maybe different (return) types of expressions?
      // TypeV type;

      /// Destructor
      explicit Expression(const rapidjson::Value &js) {}

      /// Destructor
      virtual ~Expression() = default;

      /// Evaluate the expression by interpreting it.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual Roaring *evaluate(const SequenceStore&, const MetaStore&) {
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

      explicit VectorEx(const rapidjson::Value &js) : Expression(js) {
         assert(js.HasMember("children"));
         assert(js["children"].IsArray());
         std::transform(js["children"].GetArray().begin(), js["children"].GetArray().end(),
                        std::back_inserter(children), to_ex);
      }
   };

   struct AndEx : public VectorEx {
      std::unique_ptr<Expression> start;

      explicit AndEx(const rapidjson::Value &js) : VectorEx(js) {
         start = std::move(children.back());
         children.pop_back();
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct OrEx : public VectorEx {
      explicit OrEx(const rapidjson::Value &js) : VectorEx(js) {}

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NOfEx : public VectorEx {
      unsigned n;
      bool exactly;

      explicit NOfEx(const rapidjson::Value &js) : VectorEx(js) {
         n = js["n"].GetUint();
         exactly = js["exactly"].GetBool();
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NegEx : public Expression {
      std::unique_ptr<Expression> child;

      explicit NegEx(const rapidjson::Value &js) : Expression(js) {
         child = to_ex(js["child"]);
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct DateBetwEx : public Expression {
      std::string from;
      bool open_from;
      std::string to;
      bool open_to;

      explicit DateBetwEx(const rapidjson::Value  &js) : Expression(js) {
         if (js["from"].IsNull()) {
            open_from = true;
            from = "";
         } else {
            open_from = false;
            to = js["from"].GetString();
         }

         if (js["to"].IsNull()) {
            open_to = true;
            to = "";
         } else {
            open_to = false;
            to = js["to"].GetString();
         }
      }
   };

   struct NucEqEx : public Expression {
      unsigned position;
      Symbol value;

      explicit NucEqEx(const rapidjson::Value &js) : Expression(js) {
         position = js["position"].GetUint();
         value = to_symbol(js["value"].GetString()[0]);
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NucMbEx : public Expression {
      unsigned position;
      Symbol value;

      explicit NucMbEx(const rapidjson::Value &js) : Expression(js) {
         position = js["position"].GetUint();
         value = to_symbol(js["value"].GetString()[0]);
      }
   };

   struct NucMutEx : public Expression {
      unsigned position;

      explicit NucMutEx(const rapidjson::Value &js) : Expression(js) {
         position = js["position"].GetUint();
      }

   };

   struct PangoLineageEx : public Expression {
      std::string value;
      bool includeSubLineages;

      explicit PangoLineageEx(const rapidjson::Value &js) : Expression(js) {
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


   std::unique_ptr<Expression> to_ex(const rapidjson::Value& js){
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
         throw std::runtime_error("Undefined type. Change this later to a parse exception.");
      }
   }


   Roaring *AndEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
      auto ret = start->evaluate(db, mdb);
      for (auto &child: children) {
         auto bm = child->evaluate(db, mdb);
         *ret &= *bm;
         delete bm;
      }
      return ret;
   }

   Roaring *OrEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
      unsigned n = children.size();
      const Roaring *child_res[n];
      for (int i = 0; i < n; i++) {
         child_res[i] = children[i]->evaluate(db, mdb);
      }
      auto ret = new Roaring(Roaring::fastunion(children.size(), child_res));
      for (int i = 0; i < n; i++) {
         delete child_res[i];
      }
      return ret;
   }

   Roaring *NOfEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
      std::vector<uint16_t> count;
      count.resize(db.sequenceCount);
      for (auto &child: children) {
         auto bm = child->evaluate(db, mdb);
         for (uint32_t id: *bm) {
            count[id]++;
         }
         delete bm;
      }
      std::vector<uint32_t> correct;

      if (exactly) {
         for (int i = 0; i < db.sequenceCount; i++) {
            if (count[i] == n) {
               correct.push_back(i);
            }
         }
      } else {
         for (int i = 0; i < db.sequenceCount; i++) {
            if (count[i] >= n) {
               correct.push_back(i);
            }
         }
      }

      return new Roaring(correct.size(), &correct[0]);
   }

   Roaring *NegEx::evaluate(const SequenceStore &db, const MetaStore &mdb) {
      auto ret = child->evaluate(db, mdb);
      ret->flip(0, db.sequenceCount);
      return ret;
   }

/*
Roaring* DateBetwEx::evaluate(const SequenceStore &db, const MetaStore &mdb){
   if(open_from && open_to){
      auto ret = new Roaring;
      ret->flip(0, db.sequenceCount);
      return ret;
   }
   vector<uint32_t> seqs;

   if(open_from){

   }
   else if(open_to){

   }
   else{

   }

   return new Roaring(seqs.size(), &seqs[0]);
}*/


   Roaring *NucEqEx::evaluate(const SequenceStore &db, const MetaStore &mdb) {
      return new Roaring(*db.bm(position, to_symbol(value)));
   }


   std::string execute_query(const std::string& query) {
      return "";
   }


}
