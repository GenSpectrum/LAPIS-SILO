#include "silo/query_engine/optimizer/query_plan_generator.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/ipc/writer.h>

#include "silo/query_engine/legacy_result_producer.h"

namespace silo::query_engine::optimizer {

QueryPlanGenerator::QueryPlanGenerator(std::shared_ptr<silo::Database> database)
    : database(database) {}

class OutputStreamWrapper : public arrow::io::OutputStream {
   std::ostream* output;
   bool is_closed = false;

  public:
   OutputStreamWrapper(std::ostream* output)
       : output(output) {}

   arrow::Status Close() override {
      is_closed = true;
      output->flush();
      output = nullptr;
      return arrow::Status::OK();
   }

   arrow::Status Write(const void* data, int64_t nbytes) override {
      if (is_closed) {
         return arrow::Status::Invalid("Output is already closed");
      }
      auto& x = output->write(reinterpret_cast<const char*>(data), nbytes);
      if (x.good()) {
         return arrow::Status::OK();
      }
      return arrow::Status::IOError("Failed to write to output stream: ", x.badbit);
   }

   arrow::Result<int64_t> Tell() const override {
      if (is_closed) {
         return arrow::Status::Invalid("Output is already closed");
      }
      return output->tellp();
   }

   bool closed() const override { return is_closed; }
};

class NdjsonSinkNode : public arrow::acero::ExecNode {
   std::ostream* output_stream;
   int batches_written = 0;
   std::optional<int> total_batches_from_input = std::nullopt;

  public:
   NdjsonSinkNode(arrow::acero::ExecPlan* plan, std::ostream* stream, arrow::acero::ExecNode* input)
       : arrow::acero::ExecNode(plan, {input}, {"input"}, nullptr),
         output_stream(stream) {}

   virtual const char* kind_name() const { return "NdjsonSinkNode"; }

   class ScalarToJsonTypeVisitor : public arrow::ScalarVisitor{
      std::ostream* output_stream;
     public:
      ScalarToJsonTypeVisitor(std::ostream* output_stream) : output_stream(output_stream) {}

      arrow::Status Visit(const arrow::Int32Scalar &scalar) override {
         *output_stream << scalar.value;
         return arrow::Status::OK();
      }

      arrow::Status Visit(const arrow::DoubleScalar &scalar) override {
         *output_stream << scalar.value;
         return arrow::Status::OK();
      }

      arrow::Status Visit(const arrow::StringScalar &scalar) override {
         *output_stream << "\"" << scalar.ToString() << "\"";
         return arrow::Status::OK();
      }

      arrow::Status Visit(const arrow::BooleanScalar &scalar) override {
         *output_stream << (scalar.value ? "true" : "false");
         return arrow::Status::OK();
      }

   };

   void writeRecordBatchAsNdjson(std::shared_ptr<arrow::RecordBatch> record_batch){
      arrow::Status status;
      size_t row_count = record_batch->num_rows();
      size_t column_count = record_batch->num_columns();

      std::vector<std::string> prepared_column_strings_for_json_attributes;
      for(const auto& column : record_batch->schema()->field_names()){
         prepared_column_strings_for_json_attributes.emplace_back(fmt::format("\"{}\":", column));
      }

      for(size_t row_idx = 0; row_idx < row_count; row_idx++){
         *output_stream << "{";
         for(size_t column_idx = 0; column_idx < column_count; column_idx++){
            if(column_idx != 0){
               *output_stream << ",";
            }
            const auto& column = record_batch->columns().at(column_idx);
            *output_stream << prepared_column_strings_for_json_attributes.at(column_idx);

            if(column->IsNull(row_idx)){
               *output_stream << "null";
            }
            else{
               const auto& scalar = column->GetScalar(row_idx).ValueOrDie();
               ScalarToJsonTypeVisitor my_visitor(output_stream);
               status = scalar->Accept(&my_visitor); // TODO smth with status
               if(!status.ok()){
                  throw std::runtime_error("ERR " + status.ToString());
               }
            }
         }
         *output_stream << "}\n";
      }
  }

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::compute::ExecBatch batch)
      override {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, batch.ToRecordBatch(input->output_schema()));
      writeRecordBatchAsNdjson(record_batch);
      batches_written += 1;
      if (batches_written == total_batches_from_input) {
         output_stream = nullptr;
      }
      return arrow::Status::OK();
   }

   arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) override {
      this->total_batches_from_input = total_batches;
      if (total_batches == 0) {
         output_stream = nullptr;
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

class ArrowSinkNode : public arrow::acero::ExecNode {
   OutputStreamWrapper output_stream;
   std::shared_ptr<arrow::ipc::RecordBatchWriter> writer;
   int batches_written = 0;
   std::optional<int> total_batches_from_input = std::nullopt;

  public:
   ArrowSinkNode(arrow::acero::ExecPlan* plan, std::ostream* stream, arrow::acero::ExecNode* input)
       : arrow::acero::ExecNode(plan, {input}, {"input"}, nullptr),
         output_stream(stream),
         writer(arrow::ipc::MakeStreamWriter(&output_stream, input->output_schema()).ValueOrDie()) {
   }

   const char* kind_name() const override { return "ArrowSinkNode"; };

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::compute::ExecBatch batch)
      override {
      std::shared_ptr<arrow::RecordBatch> record_batch;
      ARROW_ASSIGN_OR_RAISE(record_batch, batch.ToRecordBatch(input->output_schema()));
      ARROW_RETURN_NOT_OK(writer->WriteRecordBatch(*record_batch));
      batches_written += 1;
      if (batches_written == total_batches_from_input) {
         ARROW_RETURN_NOT_OK(writer->Close());
      }
      return arrow::Status::OK();
   }

   arrow::Status InputFinished(arrow::acero::ExecNode* input, int total_batches) override {
      this->total_batches_from_input = total_batches;
      if (total_batches == 0) {
         ARROW_RETURN_NOT_OK(writer->Close());
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducing() override { return arrow::Status::OK(); }
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

QueryPlan QueryPlanGenerator::createQueryPlan(const Query& query, std::ostream& output_stream) {
   QueryPlan query_plan;
   auto table_schema = database->schema.tables.at(schema::TableName::getDefault());
   auto output_schema =
      std::make_shared<arrow::Schema>(query.action->getOutputSchema(table_schema));
   LegacyResultProducerOptions options(output_schema, database, query);
   std::unique_ptr<arrow::acero::ExecNode> source_node =
      std::make_unique<LegacyResultProducer>(query_plan.arrow_plan.get(), options);
   std::unique_ptr<arrow::acero::ExecNode> sink_node =
      std::make_unique<NdjsonSinkNode>(query_plan.arrow_plan.get(), &output_stream, source_node.get());
   // std::make_unique<ArrowSinkNode>(query_plan.arrow_plan.get(), &output_stream, source_node.get());

   query_plan.arrow_plan->AddNode(std::move(source_node));
   query_plan.arrow_plan->AddNode(std::move(sink_node));

   return query_plan;
}

}  // namespace silo::query_engine::optimizer