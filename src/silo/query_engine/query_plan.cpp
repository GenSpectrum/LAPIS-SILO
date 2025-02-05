#include "silo/query_engine/query_plan.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "arrow/acero/query_context.h"
#include "arrow/array.h"
#include "arrow/record_batch.h"

namespace silo::query_engine {

class RecordBatchToJsonConverter : public arrow::ArrayVisitor {
   std::vector<nlohmann::json> record_batch_as_json;
   std::vector<std::string> column_names;
   size_t current_column;

  public:
   RecordBatchToJsonConverter() = default;

   arrow::Result<const std::vector<nlohmann::json>*> processBatch(arrow::RecordBatch& record_batch
   ) {
      column_names = record_batch.ColumnNames();
      current_column = 0;
      record_batch_as_json.resize(record_batch.num_rows());
      for (const auto& column : record_batch.columns()) {
         ARROW_RETURN_NOT_OK(column->Accept(this));
      }
      return arrow::Result(&record_batch_as_json);
   }

   template <typename ArrowColumnType>
   arrow::Status Visit(const ArrowColumnType& array) {
      const std::string& column_name = column_names.at(current_column);
      for (size_t i = 0; i < array.length(); ++i) {
         if (array.IsNull(i)) {
            record_batch_as_json.at(i).at(column_name) = nullptr;
         } else {
            record_batch_as_json.at(i).at(column_name) = array.Value(i);
         }
      }
      ++current_column;
      return arrow::Status::OK();
   }
};

arrow::Status writeToSinkImpl(std::ostream& output, arrow::acero::Declaration& declaration) {
   std::unique_ptr<arrow::RecordBatchReader> record_batch_reader;
   ARROW_ASSIGN_OR_RAISE(record_batch_reader, arrow::acero::DeclarationToReader(declaration));

   RecordBatchToJsonConverter visitor;
   for (auto record_batch_or_error : *record_batch_reader) {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, std::move(record_batch_or_error));

      const std::vector<nlohmann::json>* record_batch_as_json;
      ARROW_ASSIGN_OR_RAISE(record_batch_as_json, visitor.processBatch(*record_batch));

      for (const auto& row : *record_batch_as_json) {
         output << row << "\n";
      }
   }

   return arrow::Status::OK();
}

void silo::query_engine::QueryPlan::execute() {
   arrow_plan->StartProducing();
   arrow_plan->finished().Wait();
   SPDLOG_INFO("All results were produced?");
   arrow_plan->StopProducing();
}

}  // namespace silo::query_engine