#include "silo/query_engine/exec_node/legacy_result_producer.h"

#include "silo/query_engine/exec_node/arrow_util.h"

namespace silo::query_engine::exec_node {

namespace {

using filter::expressions::Expression;
using filter::operators::Operator;

QueryResult createLegacyQueryResult(const Query& query, const Database& database) {
   SPDLOG_DEBUG("Parsed query: {}", query.filter->toString());

   std::vector<std::string> compiled_queries(database.table->getNumberOfPartitions());
   std::vector<CopyOnWriteBitmap> partition_filters(database.table->getNumberOfPartitions());
   for (size_t partition_index = 0; partition_index != database.table->getNumberOfPartitions();
        partition_index++) {
      std::unique_ptr<Operator> part_filter = query.filter->compile(
         database, database.table->getPartition(partition_index), Expression::AmbiguityMode::NONE
      );
      compiled_queries[partition_index] = part_filter->toString();
      partition_filters[partition_index] = part_filter->evaluate();
   }

   for (uint32_t i = 0; i < database.table->getNumberOfPartitions(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }

   return query.action->executeAndOrder(database, std::move(partition_filters));
}
}  // namespace

JsonValueTypeArrayBuilder::JsonValueTypeArrayBuilder(std::shared_ptr<arrow::DataType> type) {
   if (type == arrow::int32()) {
      builder = arrow::Int32Builder{};
   } else if (type == arrow::float64()) {
      builder = arrow::DoubleBuilder{};
   } else if (type == arrow::utf8()) {
      builder = arrow::StringBuilder{};
   } else if (type == arrow::boolean()) {
      builder = arrow::BooleanBuilder{};
   } else {
      SILO_PANIC("Invalid type found: ", type->ToString());
   }
}

arrow::Status JsonValueTypeArrayBuilder::insert(
   const std::optional<std::variant<std::string, bool, int32_t, double>>& value
) {
   if (!value.has_value()) {
      return std::visit(
         [&](auto& b) {
            ARROW_RETURN_NOT_OK(b.AppendNull());
            return arrow::Status::OK();
         },
         builder
      );
   }

   return std::visit(
      [&](auto&& val) {
         using T = std::decay_t<decltype(val)>;

         return std::visit(
            [&](auto& b) {
               using B = std::decay_t<decltype(b)>;
               if constexpr (std::is_same_v<T, int32_t> && std::is_same_v<B, arrow::Int32Builder>) {
                  ARROW_RETURN_NOT_OK(b.Append(val));
               } else if constexpr (std::is_same_v<T, double> && std::is_same_v<B, arrow::DoubleBuilder>) {
                  ARROW_RETURN_NOT_OK(b.Append(val));
               } else if constexpr (std::is_same_v<T, std::string> && std::is_same_v<B, arrow::StringBuilder>) {
                  ARROW_RETURN_NOT_OK(b.Append(val));
               } else if constexpr (std::is_same_v<T, bool> && std::is_same_v<B, arrow::BooleanBuilder>) {
                  ARROW_RETURN_NOT_OK(b.Append(val));
               } else {
                  SILO_PANIC("Type mismatch between value and builder");
               }
               return arrow::Status::OK();
            },
            builder
         );
      },
      value.value()
   );
}

arrow::Datum JsonValueTypeArrayBuilder::toDatum() && {
   return std::visit(
      [&](auto& b) {
         using B = std::decay_t<decltype(b)>;
         if constexpr (std::is_same_v<B, arrow::Int32Builder>) {
            auto& array = get<arrow::Int32Builder>(builder);
            return arrow::Datum{array.Finish().ValueOrDie()};  // TODO
         } else if constexpr (std::is_same_v<B, arrow::DoubleBuilder>) {
            auto& array = get<arrow::DoubleBuilder>(builder);
            return arrow::Datum{array.Finish().ValueOrDie()};  // TODO
         } else if constexpr (std::is_same_v<B, arrow::StringBuilder>) {
            auto& array = get<arrow::StringBuilder>(builder);
            return arrow::Datum{array.Finish().ValueOrDie()};  // TODO
         } else if constexpr (std::is_same_v<B, arrow::BooleanBuilder>) {
            auto& array = get<arrow::BooleanBuilder>(builder);
            return arrow::Datum{array.Finish().ValueOrDie()};  // TODO
         } else {
            SILO_PANIC("Type mismatch between value and builder");
         }
      },
      builder
   );
}

LegacyResultProducer::LegacyResultProducer(
   arrow::acero::ExecPlan* plan,
   const std::vector<silo::schema::ColumnIdentifier>& columns,  // TODO const & ?
   std::shared_ptr<Database> database,
   std::shared_ptr<Query> query
)
    : arrow::acero::ExecNode(plan, {}, {}, columnsToArrowSchema(columns)) {
   query_result = createLegacyQueryResult(*query, *database);
   for (auto& field : output_schema_.get()->fields()) {
      field_names.emplace_back(&field->name());
   }
   prepareOutputArrays();
}

void LegacyResultProducer::prepareOutputArrays() {
   for (auto& field : output_schema_.get()->fields()) {
      arrays.emplace_back(field->type());
   }
}

arrow::Status LegacyResultProducer::flushOutput() {
   std::vector<arrow::Datum> data;
   for (auto& array : arrays) {
      data.push_back((std::move(array)).toDatum());
   }
   arrow::ExecBatch exec_batch;
   ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
   ARROW_RETURN_NOT_OK(this->output_->InputReceived(this, exec_batch));
   return arrow::Status::OK();
}

// TODO make configurable
static constexpr size_t MATERIALIZATION_CUTOFF = 50000;

arrow::Status LegacyResultProducer::produce() {
   size_t num_rows = 0;
   std::optional<QueryResultEntry> row;
   while ((row = query_result.next())) {
      ++num_rows;
      for (size_t field_idx = 0; field_idx < field_names.size(); ++field_idx) {
         const auto field_name = field_names.at(field_idx);
         const common::JsonValueType& field_value = row.value().fields.at(*field_name);

         auto status = arrays.at(field_idx).insert(field_value);
         if (status.IsCapacityError()) {
            throw std::runtime_error(fmt::format(
               "Response size too large. Materializing {} rows required more than allowed {} bytes",
               MATERIALIZATION_CUTOFF,
               INT32_MAX
            ));
         }
         ARROW_RETURN_NOT_OK(status);
      }

      if (num_rows > MATERIALIZATION_CUTOFF) {
         ARROW_RETURN_NOT_OK(flushOutput());
      }
   }
   ARROW_RETURN_NOT_OK(flushOutput());
   return arrow::Status::OK();
}

arrow::Status LegacyResultProducer::StartProducing() {
   return produce();
}

arrow::Status LegacyResultProducer::StopProducing() {
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
