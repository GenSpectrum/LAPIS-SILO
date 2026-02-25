#include "silo/query_engine/filter/expressions/expression.h"

#include <string>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/bool_equals.h"
#include "silo/query_engine/filter/expressions/date_between.h"
#include "silo/query_engine/filter/expressions/date_equals.h"
#include "silo/query_engine/filter/expressions/exact.h"
#include "silo/query_engine/filter/expressions/false.h"
#include "silo/query_engine/filter/expressions/float_between.h"
#include "silo/query_engine/filter/expressions/float_equals.h"
#include "silo/query_engine/filter/expressions/has_mutation.h"
#include "silo/query_engine/filter/expressions/insertion_contains.h"
#include "silo/query_engine/filter/expressions/int_between.h"
#include "silo/query_engine/filter/expressions/int_equals.h"
#include "silo/query_engine/filter/expressions/is_null.h"
#include "silo/query_engine/filter/expressions/lineage_filter.h"
#include "silo/query_engine/filter/expressions/maybe.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/nof.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/phylo_child_filter.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/expressions/string_in_set.h"
#include "silo/query_engine/filter/expressions/string_search.h"
#include "silo/query_engine/filter/expressions/symbol_equals.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"

namespace silo::query_engine::filter::expressions {

Expression::Expression() = default;

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode) {
   if (mode == Expression::UPPER_BOUND) {
      return Expression::LOWER_BOUND;
   }
   if (mode == Expression::LOWER_BOUND) {
      return Expression::UPPER_BOUND;
   }
   return mode;
}

}  // namespace silo::query_engine::filter::expressions
