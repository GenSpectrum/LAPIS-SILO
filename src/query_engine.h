//
// Created by Alexander Taepper on 27.09.22.
//

#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include "sequence_store.h"
#include <boost/json.hpp>

namespace silo {

   struct Expression {
      // For future, maybe different (return) types of expressions?
      // Type type;

      /// Destructor
      explicit Expression(const boost::json::object& js) {}
      /// Destructor
      virtual ~Expression() = default;

      /// Evaluate the expression by interpreting it.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual Roaring* evaluate(const SequenceStore& db, const MetaStore& mdb){
         throw std::runtime_error("Not Implemented exception. Does not override Expression::evaluate.");
      };

      /* Maybe generate code in the future
      /// Build the expression LLVM IR code.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual llvm::Value *build(llvm::IRBuilder<> &builder, llvm::Value *args);*/
   };

   struct VectorEx : public Expression {
      std::vector<unique_ptr<Expression>> children;

      explicit VectorEx(const boost::json::object &js) : Expression(js) {
         children = value_to< std::vector< unique_ptr<Expression> > >( js.at("children") );
      }
   };

   struct AndEx : public VectorEx {
      unique_ptr<Expression> start;

      explicit AndEx(const boost::json::object &js) : VectorEx(js) {
         start = std::move(children.back());
         children.pop_back();
      }

      Roaring* evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct OrEx : public VectorEx {
      explicit OrEx(const boost::json::object &js) : VectorEx(js) {}

      Roaring* evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NOfEx : public VectorEx {
      unsigned n;
      bool exactly;
      explicit NOfEx(const boost::json::object &js) : VectorEx(js) {
         n = value_to<unsigned>( js.at( "n" ) );
         exactly = value_to<bool>( js.at( "exactly" ) );
      }

      Roaring* evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct NegEx : public Expression {
      unique_ptr<Expression> child;
      explicit NegEx(const boost::json::object &js) : Expression(js) {
         child = value_to< unique_ptr<Expression>>(js.at("child"));
      }

      Roaring* evaluate(const SequenceStore &db, const MetaStore &mdb) override;
   };

   struct DateBetwEx : public Expression {
      string from;
      bool open_from;
      string to;
      bool open_to;
      explicit DateBetwEx(const boost::json::object &js) : Expression(js) {
         if(js.at( "from" ).is_null()){
            open_from = true;
            from = "";
         }
         else {
            open_from = false;
            from = value_to<string>(js.at("from"));
         }

         if(js.at( "to" ).is_null()){
            open_to = true;
            to = "";
         }
         else {
            open_to = false;
            to = value_to<string>( js.at( "to" ) );
         }
      }
   };

   struct NucEqEx : public Expression {
      unsigned position;
      char value;
      explicit NucEqEx(const boost::json::object &js) : Expression(js) {
         position = value_to<unsigned>( js.at( "position" ) );
         value = value_to<string>( js.at( "value" ) ).at(0);
      }
   };

   struct NucMutEx : public Expression {
      unsigned position;
      explicit NucMutEx(const boost::json::object &js) : Expression(js) {
         position = value_to<unsigned>( js.at( "position" ) );
      }

   };

   struct PangoLineageEx : public Expression {
      string value;
      bool includeSubLineages;
      explicit PangoLineageEx(const boost::json::object &js) : Expression(js) {
         includeSubLineages = value_to<bool>( js.at( "includeSubLineages" ) );
         value = value_to<string>( js.at( "value" ) );
      }

   };

   struct StrEqEx : public Expression {
      string column;
      string value;
      explicit StrEqEx(const boost::json::object &js) : Expression(js) {
         column = value_to<string>( js.at( "column" ) );
         value = value_to<string>( js.at( "value" ) );
      }

   };

   unique_ptr<Expression> tag_invoke( boost::json::value_to_tag< unique_ptr<Expression> >, const boost::json::value& jv );

}

#endif //SILO_QUERY_ENGINE_H
