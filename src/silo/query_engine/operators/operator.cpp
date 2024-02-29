#include "silo/query_engine/operators/operator.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/symbol_equals.h"
#include "silo/query_engine/operators/bitmap_producer.h"
#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/full.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/range_selection.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/query_engine/operators/threshold.h"
#include "silo/query_engine/operators/union.h"

namespace silo::query_engine::operators {

Operator::Operator() = default;

Operator::~Operator() noexcept = default;

std::unique_ptr<Operator> Operator::negate(std::unique_ptr<Operator>&& some_operator) {
   switch (some_operator->type()) {
      case EMPTY: {
         auto* empty = dynamic_cast<Empty*>(some_operator.release());
         return Empty::negate(std::unique_ptr<Empty>(empty));
      }
      case FULL: {
         auto* full = dynamic_cast<Full*>(some_operator.release());
         return Full::negate(std::unique_ptr<Full>(full));
      }
      case INDEX_SCAN: {
         auto* index_scan = dynamic_cast<IndexScan*>(some_operator.release());
         return IndexScan::negate(std::unique_ptr<IndexScan>(index_scan));
      }
      case INTERSECTION: {
         auto* intersection = dynamic_cast<Intersection*>(some_operator.release());
         return Intersection::negate(std::unique_ptr<Intersection>(intersection));
      }
      case COMPLEMENT: {
         auto* complement = dynamic_cast<Complement*>(some_operator.release());
         return Complement::negate(std::unique_ptr<Complement>(complement));
      }
      case RANGE_SELECTION: {
         auto* range_selection = dynamic_cast<RangeSelection*>(some_operator.release());
         return RangeSelection::negate(std::unique_ptr<RangeSelection>(range_selection));
      }
      case SELECTION: {
         auto* selection = dynamic_cast<Selection*>(some_operator.release());
         return Selection::negate(std::unique_ptr<Selection>(selection));
      }
      case BITMAP_SELECTION: {
         auto* bitmap_selection = dynamic_cast<BitmapSelection*>(some_operator.release());
         return BitmapSelection::negate(std::unique_ptr<BitmapSelection>(bitmap_selection));
      }
      case THRESHOLD: {
         auto* threshold = dynamic_cast<Threshold*>(some_operator.release());
         return Threshold::negate(std::unique_ptr<Threshold>(threshold));
      }
      case UNION: {
         auto* union_operator = dynamic_cast<Union*>(some_operator.release());
         return Union::negate(std::unique_ptr<Union>(union_operator));
      }
      case BITMAP_PRODUCER: {
         auto* bitmap_producer = dynamic_cast<BitmapProducer*>(some_operator.release());
         return BitmapProducer::negate(std::unique_ptr<BitmapProducer>(bitmap_producer));
      }
   }
}

std::optional<std::unique_ptr<filter_expressions::Expression>> Operator::logicalEquivalent() const {
   return std::nullopt;
}

}  // namespace silo::query_engine::operators
