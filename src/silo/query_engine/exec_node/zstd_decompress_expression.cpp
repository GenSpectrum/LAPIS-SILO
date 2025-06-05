#include "silo/query_engine/exec_node/zstd_decompress_expression.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/acero/query_context.h>
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <arrow/compute/expression.h>
#include <arrow/compute/kernel.h>
#include <arrow/compute/registry.h>
#include <arrow/memory_pool.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/type.h>
#include <arrow/type_traits.h>
#include <spdlog/spdlog.h>

#include "silo/common/panic.h"
#include "silo/zstd/zstd_context.h"
#include "silo/zstd/zstd_decompressor.h"
#include "silo/zstd/zstd_dictionary.h"

namespace silo::query_engine::exec_node {

namespace {
struct BinaryDecompressKernel {
   static arrow::Status Exec(
      arrow::compute::KernelContext* context,
      const arrow::compute::ExecSpan& input,
      arrow::compute::ExecResult* out
   ) {
      SPDLOG_INFO("BinaryDecompressKernel::Exec called!");
      if (input.num_values() != 2) {
         return arrow::Status::Invalid("Expected 2 input arrays, got ", input.num_values());
      }

      if (!input.values[0].is_array()) {
         return arrow::Status::Invalid("Expected array input, got scalar/chunked array");
      }
      const arrow::ArraySpan& input_span = input.values[0].array;
      if (input_span.type->id() != arrow::Type::BINARY) {
         return arrow::Status::Invalid(fmt::format(
            "Expected string array input, got another type: {}", input_span.type->ToString()
         ));
      }
      // TODO(#791) this is a copy of the Array's data, whereas the view should suffice
      auto array = input_span.ToArray();
      auto input_as_array = static_cast<const arrow::BinaryArray*>(array.get());
      if (!input.values[1].is_scalar()) {
         return arrow::Status::Invalid("Expected scalar input of type binary as second argument");
      }
      auto input_dict = static_cast<const arrow::BinaryScalar*>(input.values[1].scalar);
      SILO_ASSERT(input_dict);
      auto dictionary = std::make_shared<silo::ZstdDDictionary>(input_dict->view());
      silo::ZstdDecompressor decompressor{dictionary};

      arrow::StringBuilder builder(context->memory_pool());
      ARROW_RETURN_NOT_OK(builder.Reserve(input_span.length));

      for (int64_t i = 0; i < input_span.length; ++i) {
         if (input_span.IsNull(i)) {
            ARROW_RETURN_NOT_OK(builder.AppendNull());
         } else {
            const std::string_view value = input_as_array->Value(i);
            std::string decompressed_buffer;
            try {
               decompressor.decompress(value.data(), value.length(), decompressed_buffer);
            } catch (const std::exception& exception) {
               SPDLOG_ERROR("Arrow execution exception: {}", exception.what());
               return arrow::Status::ExecutionError(exception.what());
            }
            ARROW_RETURN_NOT_OK(builder.Append(decompressed_buffer));
         }
      }

      // Finish building the output array
      std::shared_ptr<arrow::Array> output_array;
      ARROW_RETURN_NOT_OK(builder.Finish(&output_array));

      // Set the output Datum
      *out = arrow::compute::ExecResult{.value = output_array->data()};
      return arrow::Status::OK();
   }
};

arrow::Result<std::string> RegisterCustomFunctionImpl() {
   std::string function_name = "silo_zstd_decompressor";
   auto registry = arrow::compute::GetFunctionRegistry();

   std::string summary =
      "Decompresses each value of zstd_compressed_binary using the scalar dictionary";
   std::string description =
      "Decompresses each value of zstd_compressed_binary using the scalar dictionary";
   std::vector<std::string> arg_names = {"zstd_compressed_binary", "dictionary"};

   // Create a new ScalarFunction
   auto custom_function = std::make_shared<arrow::compute::ScalarFunction>(
      function_name,
      arrow::compute::Arity::Binary(),
      arrow::compute::FunctionDoc(summary, description, arg_names)
   );

   {
      arrow::compute::ScalarKernel kernel;
      kernel.exec = BinaryDecompressKernel::Exec;
      kernel.signature =
         arrow::compute::KernelSignature::Make({arrow::binary(), arrow::binary()}, arrow::utf8());
      kernel.null_handling = arrow::compute::NullHandling::INTERSECTION;
      kernel.mem_allocation = arrow::compute::MemAllocation::NO_PREALLOCATE;
      ARROW_RETURN_NOT_OK(custom_function->AddKernel(std::move(kernel)));
   }

   ARROW_RETURN_NOT_OK(registry->AddFunction(custom_function));

   return function_name;
}

std::string RegisterCustomFunction() {
   std::string function_name;
   auto status = RegisterCustomFunctionImpl().Value(&function_name);
   if (!status.ok()) {
      throw std::runtime_error(fmt::format("Err: {}", status.ToString()));
   }
   return function_name;
}
}  // namespace

arrow::Expression ZstdDecompressExpression::Make(
   arrow::Expression input_expression,
   std::string dictionary_string
) {
   {
      static std::string function_name = RegisterCustomFunction();

      auto dict_scalar = std::make_shared<arrow::BinaryScalar>(std::move(dictionary_string));

      return arrow::compute::call(
         function_name, {input_expression, arrow::compute::literal(arrow::Datum(dict_scalar))}
      );
   }
}

}  // namespace silo::query_engine::exec_node