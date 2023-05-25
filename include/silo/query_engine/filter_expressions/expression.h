#ifndef SILO_EXPRESSION_H
#define SILO_EXPRESSION_H

#include <memory>
#include <string>

namespace silo::query_engine::operators {
struct Operator;
}
namespace silo {
struct Database;
struct DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

struct Expression {
   Expression();
   virtual ~Expression() = default;

   virtual std::string toString(const silo::Database& database) = 0;

   [[nodiscard]] virtual std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const = 0;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_EXPRESSION_H
