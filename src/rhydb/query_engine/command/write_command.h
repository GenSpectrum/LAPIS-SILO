#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <nlohmann/json_fwd.hpp>

#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/saneql/function_registry.h"

namespace rhydb {
class Database;
}

namespace rhydb::query_engine::command {

class WriteCommand {
  public:
   virtual ~WriteCommand() = default;

   /// Applies the command to `database`, mutating it, and returns a JSON summary of the effect
   /// (e.g. `{"insertedRows": 42}`). Consuming: a command is executed at most once.
   [[nodiscard]] virtual nlohmann::json execute(Database& database) = 0;
};

using WriteCommandPtr = std::unique_ptr<WriteCommand>;

using Request = std::variant<operators::QueryNodePtr, WriteCommandPtr>;

using WriteStatementHandler = std::function<WriteCommandPtr(
   const saneql::BoundArguments& args,
   const saneql::Tables& tables,
   const saneql::ChildConverter& convert_child
)>;

class WriteStatementRegistry {
  public:
   struct Entry {
      saneql::FunctionSignature signature;
      WriteStatementHandler handler;
   };

   WriteStatementRegistry();

   void registerStatement(
      std::string name,
      saneql::FunctionSignature signature,
      WriteStatementHandler handler
   );

   [[nodiscard]] const Entry* findStatement(const std::string& name) const;

   [[nodiscard]] static WriteStatementRegistry& instance();

  private:
   std::map<std::string, Entry> entries_;
};

/// Parses `query_string` and classifies it: if its root names a registered write statement, returns
/// the built WriteCommand; otherwise converts it to a read QueryNode tree. Throws
/// IllegalQueryException / ParseException for malformed input
Request parseRequest(std::string_view query_string, const saneql::Tables& tables);

}  // namespace rhydb::query_engine::command
