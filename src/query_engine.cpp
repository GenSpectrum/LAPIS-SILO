

#include <boost/json/src.hpp>
#include "silo/query_engine.h"

namespace silo {

   struct Expression {
      // For future, maybe different (return) types of expressions?
      // TypeV type;

      /// Destructor
      explicit Expression(const boost::json::object &js) {}

      /// Destructor
      virtual ~Expression() = default;

      /// Evaluate the expression by interpreting it.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) {
         throw std::runtime_error("Not Implemented exception. Does not override Expression::evaluate.");
      };

      /* Maybe generate code in the future
      /// Build the expression LLVM IR code.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual llvm::Value *build(llvm::IRBuilder<> &builder, llvm::Value *args);*/
   };

   struct VectorEx : public Expression {
      std::vector<std::unique_ptr<Expression>> children;

      explicit VectorEx(const boost::json::object &js) : Expression(js) {
         children = value_to<std::vector<std::unique_ptr<Expression> > >(js.at("children"));
      }
   };

   struct AndEx : public VectorEx {
      std::unique_ptr<Expression> start;

      explicit AndEx(const boost::json::object &js) : VectorEx(js) {
         start = std::move(children.back());
         children.pop_back();
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct OrEx : public VectorEx {
      explicit OrEx(const boost::json::object &js) : VectorEx(js) {}

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NOfEx : public VectorEx {
      unsigned n;
      bool exactly;

      explicit NOfEx(const boost::json::object &js) : VectorEx(js) {
         n = value_to<unsigned>(js.at("n"));
         exactly = value_to<bool>(js.at("exactly"));
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NegEx : public Expression {
      std::unique_ptr<Expression> child;

      explicit NegEx(const boost::json::object &js) : Expression(js) {
         child = value_to<std::unique_ptr<Expression>>(js.at("child"));
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct DateBetwEx : public Expression {
      std::string from;
      bool open_from;
      std::string to;
      bool open_to;

      explicit DateBetwEx(const boost::json::object &js) : Expression(js) {
         if (js.at("from").is_null()) {
            open_from = true;
            from = "";
         } else {
            open_from = false;
            from = value_to<std::string>(js.at("from"));
         }

         if (js.at("to").is_null()) {
            open_to = true;
            to = "";
         } else {
            open_to = false;
            to = value_to<std::string>(js.at("to"));
         }
      }
   };

   struct NucEqEx : public Expression {
      unsigned position;
      Symbol value;

      explicit NucEqEx(const boost::json::object &js) : Expression(js) {
         position = value_to<unsigned>(js.at("position"));
         value = to_symbol(value_to<std::string>(js.at("value")).at(0));
      }

      Roaring *evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NucMbEx : public Expression {
      unsigned position;
      Symbol value;

      explicit NucMbEx(const boost::json::object &js) : Expression(js) {
         position = value_to<unsigned>(js.at("position"));
         value = to_symbol(value_to<std::string>(js.at("value")).at(0));
      }
   };

   struct NucMutEx : public Expression {
      unsigned position;

      explicit NucMutEx(const boost::json::object &js) : Expression(js) {
         position = value_to<unsigned>(js.at("position"));
      }

   };

   struct PangoLineageEx : public Expression {
      std::string value;
      bool includeSubLineages;

      explicit PangoLineageEx(const boost::json::object &js) : Expression(js) {
         includeSubLineages = value_to<bool>(js.at("includeSubLineages"));
         value = value_to<std::string>(js.at("value"));
      }

   };

   struct StrEqEx : public Expression {
      std::string column;
      std::string value;

      explicit StrEqEx(const boost::json::object &js) : Expression(js) {
         column = value_to<std::string>(js.at("column"));
         value = value_to<std::string>(js.at("value"));
      }

   };


   std::unique_ptr<Expression> tag_invoke(boost::json::value_to_tag<std::unique_ptr<Expression>>, boost::json::value const &jv) {
      boost::json::object const &js = jv.as_object();
      auto type = value_to<std::string>(js.at("type"));
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
