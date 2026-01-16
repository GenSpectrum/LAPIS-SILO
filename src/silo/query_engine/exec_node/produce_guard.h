#pragma once

#include <arrow/util/future.h>
#include <spdlog/spdlog.h>

namespace silo::query_engine::exec_node {

class ProduceGuard {
  public:
   arrow::Future<> future = arrow::Future<>::Make();
   ProduceGuard() = default;
   ~ProduceGuard() { SPDLOG_ERROR("FREE CALLED ON PRODUCE_GUARD: {}", static_cast<void*>(this)); };
   ProduceGuard(const ProduceGuard& other) = delete;
   ProduceGuard(ProduceGuard&&) = delete;
   ProduceGuard& operator=(const ProduceGuard& other) = delete;
   ProduceGuard& operator=(ProduceGuard&&) = delete;
};

}  // namespace silo::query_engine::exec_node
