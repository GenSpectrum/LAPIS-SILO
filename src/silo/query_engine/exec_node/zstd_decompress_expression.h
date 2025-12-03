#pragma once

#include <arrow/compute/expression.h>

namespace silo::query_engine::exec_node {

class ZstdDecompressExpression {
  public:
   static arrow::compute::Expression make(
      arrow::compute::Expression input_expression,
      std::string dictionary_string
   );
};

}  // namespace silo::query_engine::exec_node
