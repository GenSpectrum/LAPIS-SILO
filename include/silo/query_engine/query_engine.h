#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include <string>
#include <variant>
#include "query_result.h"
#include "silo/database.h"

namespace silo {

struct QueryParseException : public std::exception {
  private:
   const char* message;

  public:
   explicit QueryParseException(const std::string& msg)
       : message(msg.c_str()) {}

   [[nodiscard]] const char* what() const noexcept override { return message; }
};

struct QueryResult {
   std::variant<
      response::AggregationResult,
      std::vector<response::MutationProportion>,
      response::ErrorResult>
      queryResult;
   int64_t parseTime;
   int64_t filterTime;
   int64_t actionTime;
};

/// The return value of the BoolExpression::evaluate method.
/// May return either a mutable or immutable bitmap.
struct filter_t {
   roaring::Roaring* mutable_res;
   const roaring::Roaring* immutable_res;

   inline const roaring::Roaring* getAsConst() const {
      return mutable_res ? mutable_res : immutable_res;
   }

   inline void free() {
      if (mutable_res)
         delete mutable_res;
   }
};

enum ExType { AND, OR, NOF, NEG, INDEX_FILTER, FILTER, EMPTY, FULL };

struct BoolExpression {
   // For future, maybe different (return) types of expressions?
   // TypeV type;

   BoolExpression() {}

   /// Destructor
   virtual ~BoolExpression() = default;

   virtual ExType type() const = 0;

   /// Evaluate the expression by interpreting it.
   /// If mutable bitmap is returned, caller must free the result
   virtual filter_t evaluate(const Database& /*db*/, const DatabasePartition& /*dbp*/) = 0;

   /// Transforms the expression to a human readable string.
   virtual std::string to_string(const Database& db) = 0;

   virtual std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const = 0;

   /* Maybe generate code in the future
      /// Build the expression LLVM IR code.
      /// @args: all function arguments that can be referenced by an @Argument
      virtual llvm::Value *build(llvm::IRBuilder<> &builder, llvm::Value *args);*/
};

struct EmptyEx : public BoolExpression {
   ExType type() const override { return ExType::EMPTY; };

   /// EmptyEx should be simplified away.
   filter_t evaluate(const Database& /*db*/, const DatabasePartition& /*dbp*/) override;

   std::string to_string(const Database& /*db*/) override { return "FALSE"; }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<silo::EmptyEx>();
   }
};

struct FullEx : public BoolExpression {
   ExType type() const override { return ExType::FULL; };

   /// EmptyEx should be simplified away.
   filter_t evaluate(const Database& /*db*/, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override { return "TRUE"; }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<silo::FullEx>();
   }
};

struct AndEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   std::vector<std::unique_ptr<BoolExpression>> negated_children;

   explicit AndEx() {}

   ExType type() const override { return ExType::AND; };

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "(";
      for (auto& child : children) {
         res += " & ";
         res += child->to_string(db);
      }
      for (auto& child : negated_children) {
         res += " &! ";
         res += child->to_string(db);
      }
      res += ")";
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp)
      const override;
};

struct OrEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;

   explicit OrEx() {}

   ExType type() const override { return ExType::OR; };

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

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp)
      const override;
};

struct NOfEx : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   unsigned n;
   unsigned impl;
   bool exactly;

   ExType type() const override { return ExType::NOF; };

   explicit NOfEx(unsigned n, unsigned impl, bool exactly)
       : n(n),
         impl(impl),
         exactly(exactly) {}

   explicit NOfEx(unsigned n, bool exactly)
       : n(n),
         impl(1),
         exactly(exactly) {}

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

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp)
      const override;
};

struct NegEx : public BoolExpression {
   std::unique_ptr<BoolExpression> child;

   ExType type() const override { return ExType::NEG; };

   explicit NegEx() {}

   explicit NegEx(std::unique_ptr<BoolExpression> child)
       : child(std::move(child)) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "!" + child->to_string(db);
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& db, const DatabasePartition& dbp)
      const override;
};

struct NucEqEx : public BoolExpression {
   unsigned position;
   Symbol value;
   bool individualized = false;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit NucEqEx() {}

   explicit NucEqEx(unsigned position, Symbol value)
       : position(position),
         value(value) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = std::to_string(position) + symbol_rep[value];
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp)
      const override;
};

struct NucMbEx : public BoolExpression {
   unsigned position;
   Symbol value;
   bool negated = false;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit NucMbEx() {}

   explicit NucMbEx(unsigned position, Symbol value)
       : position(position),
         value(value) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = "?" + std::to_string(position) + symbol_rep[value];
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp)
      const override;
};

struct PangoLineageEx : public BoolExpression {
   uint32_t lineageKey;
   bool includeSubLineages;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit PangoLineageEx(uint32_t lineageKey, bool includeSubLineages)
       : lineageKey(lineageKey),
         includeSubLineages(includeSubLineages) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = db.dict->get_pango(lineageKey);
      if (includeSubLineages) {
         res += ".*";
      }
      return res;
   }

   std::unique_ptr<BoolExpression> simplify(const Database& /*db*/, const DatabasePartition& dbp)
      const override;
};

struct CountryEx : public BoolExpression {
   uint32_t countryKey;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit CountryEx(uint32_t countryKey)
       : countryKey(countryKey) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "Country=" + db.dict->get_country(countryKey);
      return res;
   }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override;
};

struct RegionEx : public BoolExpression {
   uint32_t regionKey;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit RegionEx(uint32_t regionKey)
       : regionKey(regionKey) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   std::string to_string(const Database& db) override {
      std::string res = "Region=" + db.dict->get_region(regionKey);
      return res;
   }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override;
};

struct SelectEx : public BoolExpression {
   virtual filter_t select(
      const Database& db,
      const DatabasePartition& dbp,
      filter_t in_filter
   ) = 0;
   virtual filter_t neg_select(
      const Database& db,
      const DatabasePartition& dbp,
      filter_t in_filter
   ) = 0;
};

struct DateBetwEx : public SelectEx {
   time_t from;
   bool open_from;
   time_t to;
   bool open_to;

   ExType type() const override { return ExType::INDEX_FILTER; };

   explicit DateBetwEx() {}

   explicit DateBetwEx(time_t from, bool open_from, time_t to, bool open_to)
       : from(from),
         open_from(open_from),
         to(to),
         open_to(open_to) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   filter_t select(const Database& db, const DatabasePartition& dbp, filter_t in_filter) override;

   filter_t neg_select(const Database& db, const DatabasePartition& dbp, filter_t in_filter)
      override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = "[Date-between ";
      res += (open_from ? "unbound" : std::to_string(from));
      res += " and ";
      res += (open_to ? "unbound" : std::to_string(to));
      res += "]";
      return res;
   }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<DateBetwEx>(from, open_from, to, open_to);
   }
};

struct PosNEqEx : public SelectEx {
   unsigned position;

   ExType type() const override { return ExType::FILTER; };

   explicit PosNEqEx(unsigned position)
       : position(position) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   filter_t select(const Database& db, const DatabasePartition& dbp, filter_t in_filter) override;

   filter_t neg_select(const Database& db, const DatabasePartition& dbp, filter_t in_filter)
      override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = std::to_string(position) + "N";
      return res;
   }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      return std::make_unique<PosNEqEx>(position);
   }
};

struct StrEqEx : public SelectEx {
   uint32_t column;
   uint64_t value;

   ExType type() const override { return ExType::FILTER; };

   explicit StrEqEx(uint32_t column, uint64_t value)
       : column(column),
         value(value) {}

   filter_t evaluate(const Database& db, const DatabasePartition& dbp) override;

   filter_t select(const Database& db, const DatabasePartition& dbp, filter_t in_filter) override;

   filter_t neg_select(const Database& db, const DatabasePartition& dbp, filter_t in_filter)
      override;

   std::string to_string(const Database& /*db*/) override {
      std::string res = column + "=" + value;
      return res;
   }

   std::unique_ptr<BoolExpression>
   simplify(const Database& /*db*/, const DatabasePartition& /*dbp*/) const override {
      if (column == UINT32_MAX || value == UINT64_MAX) {
         return std::make_unique<EmptyEx>();
      }
      return std::make_unique<StrEqEx>(column, value);
   }
};

struct MutationProportion {
   char mut_from;
   unsigned position;
   char mut_to;
   double proportion;
   unsigned count;
};

/// Filter then call action
QueryResult execute_query(
   const Database& db,
   const std::string& query,
   std::ostream& parse_out,
   std::ostream& perf_out
);

std::vector<silo::filter_t> execute_predicate(
   const silo::Database& db,
   const BoolExpression* filter
);

/// Action
std::vector<MutationProportion> execute_mutations(
   const silo::Database&,
   std::vector<silo::filter_t>&,
   double proportion_threshold,
   std::ostream& performance_file
);

std::vector<std::vector<uint32_t>> execute_all_dist(
   const silo::Database& db,
   std::vector<silo::filter_t>& partition_filters
);

uint64_t execute_count(
   const silo::Database& /*db*/,
   std::vector<silo::filter_t>& partition_filters
);

}  // namespace silo

#endif  // SILO_QUERY_ENGINE_H
