#include "silo/query_engine/exec_node/json_value_type_array_builder.h"

#include <arrow/datum.h>

#include "silo/common/panic.h"

namespace silo::query_engine::exec_node {

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

arrow::Result<arrow::Datum> JsonValueTypeArrayBuilder::toDatum() {
   return std::visit(
             [&](auto& b) {
                using B = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<B, arrow::Int32Builder>) {
                   auto& array = get<arrow::Int32Builder>(builder);
                   return array.Finish();
                } else if constexpr (std::is_same_v<B, arrow::DoubleBuilder>) {
                   auto& array = get<arrow::DoubleBuilder>(builder);
                   return array.Finish();
                } else if constexpr (std::is_same_v<B, arrow::StringBuilder>) {
                   auto& array = get<arrow::StringBuilder>(builder);
                   return array.Finish();
                } else if constexpr (std::is_same_v<B, arrow::BooleanBuilder>) {
                   auto& array = get<arrow::BooleanBuilder>(builder);
                   return array.Finish();
                } else {
                   SILO_PANIC("Type mismatch between value and builder");
                }
             },
             builder
   )
      .Map([](auto&& x) { return arrow::Datum{x}; });
}

}  // namespace silo::query_engine::exec_node
