#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/saneql/ast.h"
#include "silo/storage/table.h"

namespace silo::query_engine::saneql {

using Tables = std::map<schema::TableName, std::shared_ptr<storage::Table>>;

struct ParameterDefinition {
   std::string name;
   bool required = true;
   /// If false, the parameter can only be filled via named argument syntax.
   bool positional = true;
};

struct FunctionSignature {
   std::vector<ParameterDefinition> parameters;
};

/// Result of binding a FunctionCall's arguments against a FunctionSignature.
class BoundArguments {
  public:
   BoundArguments(std::string function_name, std::map<std::string, const ast::Expression*> bound);

   /// Returns the expression for a required parameter. Throws if absent.
   [[nodiscard]] const ast::Expression& at(const std::string& name) const;

   /// Returns the expression for an optional parameter, or nullptr if absent.
   [[nodiscard]] const ast::Expression* get(const std::string& name) const;

   [[nodiscard]] bool has(const std::string& name) const;

   [[nodiscard]] const std::string& functionName() const;

   [[nodiscard]] std::optional<std::string> getOptionalString(const std::string& name) const;

   [[nodiscard]] std::optional<uint32_t> getOptionalUint32(const std::string& name) const;

  private:
   std::string function_name_;
   std::map<std::string, const ast::Expression*> bound_;
};

/// Match positional then named arguments against a signature.
/// Errors on: too many positional args (non-variadic), unknown named args,
/// duplicate bindings, missing required parameters.
BoundArguments bindArguments(
   const std::string& function_name,
   const FunctionSignature& signature,
   const std::vector<ast::PositionalArgument>& positional,
   const std::vector<ast::NamedArgument>& named
);

// --- Pipeline function registry ---

using ChildConverter =
   std::function<operators::QueryNodePtr(const ast::Expression&, const Tables&)>;

using FunctionHandler = std::function<operators::QueryNodePtr(
   const BoundArguments& args,
   const Tables& tables,
   const ChildConverter& convert_child
)>;

class FunctionRegistry {
  public:
   struct Entry {
      FunctionSignature signature;
      FunctionHandler handler;
   };

   FunctionRegistry();

   void registerFunction(std::string name, FunctionSignature signature, FunctionHandler handler);

   [[nodiscard]] const Entry* findFunction(const std::string& name) const;

   [[nodiscard]] static FunctionRegistry& instance();

  private:
   std::map<std::string, Entry> entries_;
};

// --- Filter function registry ---

using FilterPtr = std::unique_ptr<query_engine::filter::expressions::Expression>;

using FilterHandler = std::function<FilterPtr(const BoundArguments& args)>;

class FilterFunctionRegistry {
  public:
   struct Entry {
      FunctionSignature signature;
      FilterHandler handler;
   };

   FilterFunctionRegistry();

   void registerFunction(std::string name, FunctionSignature signature, FilterHandler handler);

   [[nodiscard]] const Entry* findFunction(const std::string& name) const;

   [[nodiscard]] static FilterFunctionRegistry& instance();

  private:
   std::map<std::string, Entry> entries_;
};

}  // namespace silo::query_engine::saneql
