#include "silo/query_engine/optimizer/query_plan_generator.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>

#include "silo/query_engine/legacy_result_producer.h"

namespace silo::query_engine::optimizer {

namespace {
void doInitialization() {
   arrow::acero::ExecFactoryRegistry::Factory factory =
      [](arrow::acero::ExecPlan* plan,
         std::vector<arrow::acero::ExecNode*> inputs,
         const arrow::acero::ExecNodeOptions& options) {
         const auto& real_options = *static_cast<const LegacyResultProducerOptions*>(&options);
         LegacyResultProducer producer{plan, real_options};
         return arrow::Result<arrow::acero::ExecNode*>(&producer);
      };
   auto registry = arrow::acero::default_exec_factory_registry();
   auto register_new_factory_status = registry->AddFactory("legacyProducer", factory);
   if (!register_new_factory_status.ok()) {
      throw std::runtime_error(register_new_factory_status.ToString());
   }
}
}  // namespace

QueryPlanGenerator::QueryPlanGenerator(std::shared_ptr<silo::Database> database)
    : database(database) {
   // Every request will create its separate QueryEngine object, so this must be thread-safe
   static std::once_flag flag;
   std::call_once(flag, doInitialization);
}

QueryPlan QueryPlanGenerator::createQueryPlan(const Query& query) {
   arrow::acero::ExecPlan plan;
   auto table_schema = database->schema.tables.at(schema::TableName::getDefault());
   auto output_schema =
      std::make_shared<arrow::Schema>(query.action->getOutputSchema(table_schema));
   std::shared_ptr<arrow::acero::ExecNodeOptions> options =
      std::make_shared<LegacyResultProducerOptions>(output_schema, database, query);
   arrow::acero::Declaration declaration =
      arrow::acero::Declaration("legacyProducer", options, "default_label");

   QueryPlan query_plan;
   query_plan.declaration = declaration;
   return query_plan;
}

}  // namespace silo::query_engine::optimizer