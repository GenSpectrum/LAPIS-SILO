#include "silo/query_engine/operator_result.h"

namespace silo::query_engine {

[[nodiscard]] const roaring::Roaring* OperatorResult::getAsConst() const {
   return mutable_res ? mutable_res : immutable_res;
}

void OperatorResult::free() const {
   delete mutable_res;
}

}  // namespace silo::query_engine
