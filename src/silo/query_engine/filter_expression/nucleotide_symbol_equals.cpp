#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"

#include <vector>

#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

NucleotideSymbolEquals::NucleotideSymbolEquals(unsigned int position, silo::NUCLEOTIDE_SYMBOL value)
    : position(position),
      value(value) {}

std::string NucleotideSymbolEquals::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(position) + genomeSymbolRepresentation(value);
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> NucleotideSymbolEquals::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition
) const {
   if (value == NUCLEOTIDE_SYMBOL::N && !database_partition.seq_store.positions[position].nucleotide_symbol_n_indexed) {
      return std::make_unique<operators::BitmapSelection>(
         database_partition.seq_store.nucleotide_symbol_n_bitmaps.data(),
         database_partition.seq_store.nucleotide_symbol_n_bitmaps.size(),
         operators::BitmapSelection::CONTAINS,
         position
      );
   }
   if (database_partition.seq_store.positions[position].symbol_whose_bitmap_is_flipped == value) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            database_partition.seq_store.getBitmap(position, value)
         ),
         database_partition.sequenceCount
      );
   }

   return std::make_unique<operators::IndexScan>(
      database_partition.seq_store.getBitmap(position, value)
   );
}

}  // namespace silo::query_engine::filter_expressions