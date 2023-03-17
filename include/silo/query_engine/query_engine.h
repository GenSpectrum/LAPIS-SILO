#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include <iostream>
#include <memory>
#include <roaring/roaring.hh>
#include <string>
#include <vector>

#include "silo/common/silo_symbols.h"

namespace silo {

class Database;
class DatabasePartition;

namespace response {
struct QueryResult;
}
/// The return value of the BoolExpression::evaluate method.
/// May return either a mutable or immutable bitmap.
struct BooleanExpressionResult {
   roaring::Roaring* mutable_res;
   const roaring::Roaring* immutable_res;

   [[nodiscard]] const roaring::Roaring* getAsConst() const;

   void free() const;
};

enum ExpressionType { AND, OR, NOF, NEG, INDEX_FILTER, FILTER, EMPTY, FULL };

struct BoolExpression {
   BoolExpression();
   virtual ~BoolExpression() = default;

   [[nodiscard]] virtual ExpressionType type() const = 0;
   virtual BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) = 0;

   virtual std::string toString(const Database& database) = 0;

   [[nodiscard]] virtual std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const = 0;
};

struct EmptyExpression : public BoolExpression {
   [[nodiscard]] ExpressionType type() const override;

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct FullExpression : public BoolExpression {
   [[nodiscard]] ExpressionType type() const override;

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct AndExpression : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   std::vector<std::unique_ptr<BoolExpression>> negated_children;

   explicit AndExpression();

   [[nodiscard]] ExpressionType type() const override;

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct OrExpression : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;

   explicit OrExpression();

   [[nodiscard]] ExpressionType type() const override;

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

enum NOfExpressionImplementation { GENERIC, LOOP_DATABASE_PARTITION, N_WAY_HEAP_MERGE };

struct NOfExpression : public BoolExpression {
   std::vector<std::unique_ptr<BoolExpression>> children;
   unsigned number_of_matchers;
   NOfExpressionImplementation implementation;
   bool match_exactly;

   [[nodiscard]] ExpressionType type() const override;

   explicit NOfExpression(
      unsigned number_of_matchers,
      bool match_exactly,
      NOfExpressionImplementation implementation = NOfExpressionImplementation::GENERIC
   );

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct NegatedExpression : public BoolExpression {
   std::unique_ptr<BoolExpression> child;

   [[nodiscard]] ExpressionType type() const override;

   explicit NegatedExpression();

   explicit NegatedExpression(std::unique_ptr<BoolExpression> child);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct NucleotideSymbolEqualsExpression : public BoolExpression {
   unsigned position;
   GENOME_SYMBOL value;
   bool individualized = false;

   [[nodiscard]] ExpressionType type() const override;

   explicit NucleotideSymbolEqualsExpression();

   explicit NucleotideSymbolEqualsExpression(unsigned position, GENOME_SYMBOL value);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct NucleotideSymbolMaybeExpression : public BoolExpression {
   unsigned position;
   GENOME_SYMBOL value;
   bool negated = false;

   [[nodiscard]] ExpressionType type() const override;

   explicit NucleotideSymbolMaybeExpression();

   explicit NucleotideSymbolMaybeExpression(unsigned position, GENOME_SYMBOL value);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct PangoLineageExpression : public BoolExpression {
   uint32_t lineageKey;
   bool include_sublineages;

   [[nodiscard]] ExpressionType type() const override;

   explicit PangoLineageExpression(uint32_t lineage_key, bool include_sublineages);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct CountryExpression : public BoolExpression {
   uint32_t country_key;

   [[nodiscard]] ExpressionType type() const override;

   explicit CountryExpression(uint32_t country_key);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct RegionExpression : public BoolExpression {
   uint32_t region_key;

   [[nodiscard]] ExpressionType type() const override;

   explicit RegionExpression(uint32_t regionKey);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct SelectExpression : public BoolExpression {
   virtual BooleanExpressionResult select(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) = 0;
   virtual BooleanExpressionResult selectNegated(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) = 0;
};

struct DateBetweenExpression : public SelectExpression {
   time_t date_from;
   bool open_from;
   time_t date_to;
   bool open_to;

   [[nodiscard]] ExpressionType type() const override;

   explicit DateBetweenExpression();

   explicit DateBetweenExpression(time_t date_from, bool open_from, time_t date_to, bool open_to);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   BooleanExpressionResult select(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   BooleanExpressionResult selectNegated(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct PositionHasNucleotideSymbolNExpression : public SelectExpression {
   unsigned position;

   [[nodiscard]] ExpressionType type() const override;

   explicit PositionHasNucleotideSymbolNExpression(unsigned position);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   BooleanExpressionResult select(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   BooleanExpressionResult selectNegated(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

struct StringEqualsExpression : public SelectExpression {
   uint32_t column;
   uint64_t value;

   [[nodiscard]] ExpressionType type() const override;

   explicit StringEqualsExpression(uint32_t column, uint64_t value);

   BooleanExpressionResult evaluate(
      const Database& database,
      const DatabasePartition& database_partition
   ) override;

   BooleanExpressionResult select(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   BooleanExpressionResult selectNegated(
      const Database& database,
      const DatabasePartition& database_partition,
      BooleanExpressionResult in_filter
   ) override;

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<BoolExpression> simplify(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

static const double DEFAULT_MINIMAL_PROPORTION = 0.02;
struct MutationProportion {
   char mutation_from;
   unsigned position;
   char mutation_to;
   double proportion;
   unsigned count;
};

response::QueryResult executeQuery(
   const Database& database,
   const std::string& query,
   std::ostream& perf_out
);

std::vector<MutationProportion> executeMutations(
   const silo::Database&,
   std::vector<silo::BooleanExpressionResult>& partition_filters,
   double proportion_threshold,
   std::ostream& performance_file
);

uint64_t executeCount(
   const silo::Database& database,
   std::vector<silo::BooleanExpressionResult>& partition_filters
);

}  // namespace silo

#endif  // SILO_QUERY_ENGINE_H
