#include "silo/query_engine/saneql/function_registry.h"

#include <set>

#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::saneql {

// --- BoundArguments ---

BoundArguments::BoundArguments(
   std::string function_name,
   std::map<std::string, const ast::Expression*> bound
)
    : function_name_(std::move(function_name)),
      bound_(std::move(bound)) {}

const ast::Expression& BoundArguments::at(const std::string& name) const {
   auto it = bound_.find(name);
   CHECK_SILO_QUERY(
      it != bound_.end(), "{}(): required argument '{}' is missing", function_name_, name
   );
   return *it->second;
}

const ast::Expression* BoundArguments::get(const std::string& name) const {
   auto it = bound_.find(name);
   if (it == bound_.end()) {
      return nullptr;
   }
   return it->second;
}

bool BoundArguments::has(const std::string& name) const {
   return bound_.contains(name);
}

const std::string& BoundArguments::functionName() const {
   return function_name_;
}

std::optional<std::string> BoundArguments::getOptionalString(const std::string& name) const {
   if (const auto* expr = get(name)) {
      return extractStringLiteral(*expr);
   }
   return std::nullopt;
}

std::optional<uint32_t> BoundArguments::getOptionalUint32(const std::string& name) const {
   if (const auto* expr = get(name)) {
      int64_t value = extractIntLiteral(*expr);
      CHECK_SILO_QUERY(
         value >= 0, "If the action contains an {}, it must be a non-negative number", name
      );
      return static_cast<uint32_t>(value);
   }
   return std::nullopt;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
BoundArguments bindArguments(
   const std::string& function_name,
   const FunctionSignature& signature,
   const std::vector<ast::PositionalArgument>& positional,
   const std::vector<ast::NamedArgument>& named
) {
   std::map<std::string, const ast::Expression*> bound;

   // Iterate through positional parameters in declaration order
   size_t next_param = 0;
   for (const auto& pos_arg : positional) {
      // Find the next parameter that accepts positional binding
      const ParameterDefinition* target = nullptr;
      while (next_param < signature.parameters.size()) {
         if (signature.parameters[next_param].positional) {
            target = &signature.parameters[next_param];
            ++next_param;
            break;
         }
         ++next_param;
      }
      CHECK_SILO_QUERY(
         target != nullptr, "{}() received too many positional arguments", function_name
      );
      bound[target->name] = pos_arg.value.get();
   }

   // Build set of valid parameter names for validation
   std::set<std::string> valid_names;
   for (const auto& param : signature.parameters) {
      valid_names.insert(param.name);
   }

   // Bind named arguments
   for (const auto& named_arg : named) {
      CHECK_SILO_QUERY(
         valid_names.contains(named_arg.name),
         "{}() received unknown argument '{}'",
         function_name,
         named_arg.name
      );
      CHECK_SILO_QUERY(
         !bound.contains(named_arg.name),
         "{}() received duplicate argument '{}' (already bound positionally)",
         function_name,
         named_arg.name
      );
      bound[named_arg.name] = named_arg.value.get();
   }

   // Check that all required parameters are bound
   for (const auto& param : signature.parameters) {
      CHECK_SILO_QUERY(
         !param.required || bound.contains(param.name),
         "{}() requires argument '{}'",
         function_name,
         param.name
      );
   }

   return {function_name, std::move(bound)};
}

// --- FunctionRegistry ---

void FunctionRegistry::registerFunction(
   std::string name,
   FunctionSignature signature,
   FunctionHandler handler
) {
   entries_[std::move(name)] =
      Entry{.signature = std::move(signature), .handler = std::move(handler)};
}

const FunctionRegistry::Entry* FunctionRegistry::findFunction(const std::string& name) const {
   auto it = entries_.find(name);
   if (it == entries_.end()) {
      return nullptr;
   }
   return &it->second;
}

// --- FilterFunctionRegistry ---

void FilterFunctionRegistry::registerFunction(
   std::string name,
   FunctionSignature signature,
   FilterHandler handler
) {
   entries_[std::move(name)] =
      Entry{.signature = std::move(signature), .handler = std::move(handler)};
}

const FilterFunctionRegistry::Entry* FilterFunctionRegistry::findFunction(const std::string& name
) const {
   auto it = entries_.find(name);
   if (it == entries_.end()) {
      return nullptr;
   }
   return &it->second;
}

}  // namespace silo::query_engine::saneql
