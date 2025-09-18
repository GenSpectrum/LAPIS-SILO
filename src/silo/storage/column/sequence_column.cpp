#include "silo/storage/column/sequence_column.h"

#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/parallel.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/common/table_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/sequence_position.h"
#include "silo/storage/insertion_format_exception.h"

namespace silo::storage::column {

template <typename SymbolType>
SequenceColumnPartition<SymbolType>::SequenceColumnPartition(
   SequenceColumnMetadata<SymbolType>* metadata
)
    : metadata(metadata) {
   lazy_buffer.reserve(BUFFER_SIZE);
   positions.reserve(metadata->reference_sequence.size());
   for (const auto symbol : metadata->reference_sequence) {
      positions.emplace_back(SequencePosition<SymbolType>::fromInitiallyFlipped(symbol));
   }
}

template <typename SymbolType>
ReadSequence& SequenceColumnPartition<SymbolType>::appendNewSequenceRead() {
   if (lazy_buffer.size() > BUFFER_SIZE) {
      flushBuffer();
      // TODO(#878) do not clear buffer but reset an offset instead
      lazy_buffer.clear();
   }

   // TODO(#878) use [offset] instead of emplace_back()
   lazy_buffer.emplace_back();

   sequence_count += 1;

   // TODO(#878) use .at(offset) instead of back()
   return lazy_buffer.back();
}

namespace {

constexpr std::string_view DELIMITER_INSERTION = ":";

template <typename SymbolType>
struct InsertionEntry {
   uint32_t position_idx;
   std::string insertion;
};

template <typename SymbolType>
InsertionEntry<SymbolType> parseInsertion(const std::string& value) {
   auto position_and_insertion = splitBy(value, DELIMITER_INSERTION);
   std::ranges::transform(
      position_and_insertion,
      position_and_insertion.begin(),
      [](const std::string& value) { return removeSymbol(value, '\"'); }
   );
   if (position_and_insertion.size() == 2 && !position_and_insertion.at(1).empty()) {
      try {
         const auto position = boost::lexical_cast<uint32_t>(position_and_insertion[0]);
         const auto& insertion = position_and_insertion[1];
         if (insertion.empty()) {
            throw silo::storage::InsertionFormatException(
               "Failed to parse insertion due to invalid format. Expected position that is "
               "parsable "
               "as an integer, instead got: '{}'",
               value
            );
         }
         for (char character : insertion) {
            auto symbol = SymbolType::charToSymbol(character);
            if (symbol == std::nullopt) {
               throw InsertionFormatException(fmt::format(
                  "Illegal {} character '{}' in insertion: {}",
                  SymbolType::SYMBOL_NAME_LOWER_CASE,
                  character,
                  value
               ));
            }
         }
         return {.position_idx = position, .insertion = insertion};
      } catch (const boost::bad_lexical_cast& error) {
         throw silo::storage::InsertionFormatException(
            "Failed to parse insertion due to invalid format. Expected position that is parsable "
            "as "
            "an integer, instead got: '{}'",
            value
         );
      }
   }
   throw silo::storage::InsertionFormatException(
      "Failed to parse insertion due to invalid format. Expected two parts (position and non-empty "
      "insertion "
      "value), instead got: '{}'",
      value
   );
}
}  // namespace

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::appendInsertion(const std::string& insertion_and_position
) {
   auto [position, insertion] = parseInsertion<SymbolType>(insertion_and_position);
   insertion_index.addLazily(position, insertion, sequence_count - 1);
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::finalize() {
   flushBuffer();
   lazy_buffer.clear();

   SPDLOG_DEBUG("Building insertion index");

   insertion_index.buildIndex();

   SPDLOG_DEBUG("Optimizing bitmaps");

   const SequenceColumnInfo info_before_optimisation = calculateInfo();
   optimizeBitmaps();

   SPDLOG_DEBUG(
      "Sequence store partition info after filling it: {}, and after optimising: {}",
      info_before_optimisation,
      calculateInfo()
   );
}
}  // namespace silo::storage::column

[[maybe_unused]] auto fmt::formatter<silo::storage::column::SequenceColumnInfo>::format(
   const silo::storage::column::SequenceColumnInfo& sequence_store_info,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
      ctx.out(),
      "SequenceColumnInfo[sequence count: {}, vertical bitmaps size: {}, horizontal bitmaps size: "
      "{}]",
      sequence_store_info.sequence_count,
      sequence_store_info.vertical_bitmaps_size,
      sequence_store_info.horizontal_bitmaps_size
   );
}

namespace silo::storage::column {

template <typename SymbolType>
SequenceColumnInfo SequenceColumnPartition<SymbolType>::calculateInfo() {
   sequence_column_info = {
      .sequence_count = sequence_count,
      .vertical_bitmaps_size = computeVerticalBitmapsSize(),
      .horizontal_bitmaps_size = computeHorizontalBitmapsSize()
   };
   return sequence_column_info;
}

template <typename SymbolType>
SequenceColumnInfo SequenceColumnPartition<SymbolType>::getInfo() const {
   return sequence_column_info;
}

template <typename SymbolType>
const roaring::Roaring* SequenceColumnPartition<SymbolType>::getBitmap(
   size_t position_idx,
   typename SymbolType::Symbol symbol
) const {
   return positions[position_idx].getBitmap(symbol);
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::fillIndexes() {
   EVOBENCH_SCOPE("SequenceColumnPartition", "fillIndexes");
   const size_t genome_length = positions.size();
   static constexpr int DEFAULT_POSITION_BATCH_SIZE = 64;
   const size_t sequence_id_base_for_buffer = sequence_count - lazy_buffer.size();
   common::parallel_for(
      common::blocked_range(0, genome_length),
      DEFAULT_POSITION_BATCH_SIZE,
      [&](const auto& local) {
         EVOBENCH_SCOPE_EVERY(100, "SequenceColumnPartition", "fillIndexes-chunk");
         SymbolMap<SymbolType, std::vector<uint32_t>> ids_per_symbol_for_current_position;
         for (size_t position_idx = local.begin(); position_idx < local.end(); ++position_idx) {
            const size_t number_of_sequences = lazy_buffer.size();
            for (size_t sequence_offset = 0; sequence_offset < number_of_sequences;
                 ++sequence_offset) {
               const auto& [is_valid, sequence, offset] = lazy_buffer[sequence_offset];
               if (!is_valid || position_idx < offset || position_idx - offset >= sequence.size()) {
                  continue;
               }
               const char character = sequence[position_idx - offset];
               const auto symbol = SymbolType::charToSymbol(character);
               if (!symbol.has_value()) {
                  throw preprocessing::PreprocessingException(
                     "Illegal character '{}' at position {} contained in sequence with index {} in "
                     "the current buffer.",
                     character,
                     position_idx,
                     sequence_offset
                  );
               }
               if (symbol != SymbolType::SYMBOL_MISSING) {
                  ids_per_symbol_for_current_position[*symbol].push_back(
                     sequence_id_base_for_buffer + sequence_offset
                  );
               }
            }
            addSymbolsToPositions(
               position_idx, ids_per_symbol_for_current_position, number_of_sequences
            );
         }
      }
   );
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::addSymbolsToPositions(
   size_t position_idx,
   SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
   size_t number_of_sequences
) {
   for (const auto& symbol : SymbolType::SYMBOLS) {
      positions[position_idx].addValues(
         symbol,
         ids_per_symbol_for_current_position.at(symbol),
         sequence_count - number_of_sequences,
         number_of_sequences
      );
      ids_per_symbol_for_current_position[symbol].clear();
   }
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::fillNBitmaps() {
   EVOBENCH_SCOPE("SequenceColumnPartition", "fillNBitmaps");
   const size_t genome_length = positions.size();

   const size_t sequence_id_base_for_buffer = sequence_count - lazy_buffer.size();

   missing_symbol_bitmaps.resize(sequence_count);

   common::parallel_for(
      common::blocked_range{0, lazy_buffer.size()},
      1,  // positions_per_process
      [&](common::blocked_range local) {
         EVOBENCH_SCOPE_EVERY(100, "SequenceColumnPartition", "fillNBitmaps-chunk");
         std::vector<uint32_t> positions_with_symbol_missing;
         for (size_t sequence_offset_in_buffer = local.begin();
              sequence_offset_in_buffer != local.end();
              ++sequence_offset_in_buffer) {
            const auto& [is_valid, maybe_sequence, offset] = lazy_buffer[sequence_offset_in_buffer];

            const size_t sequence_idx = sequence_id_base_for_buffer + sequence_offset_in_buffer;

            if (!is_valid) {
               missing_symbol_bitmaps[sequence_idx].addRange(0, genome_length);
               missing_symbol_bitmaps[sequence_idx].runOptimize();
               continue;
            }

            missing_symbol_bitmaps[sequence_idx].addRange(0, offset);

            for (size_t position_idx = 0; position_idx < maybe_sequence.size(); ++position_idx) {
               const char character = maybe_sequence[position_idx];
               const auto symbol = SymbolType::charToSymbol(character);
               if (symbol == SymbolType::SYMBOL_MISSING) {
                  positions_with_symbol_missing.push_back(position_idx + offset);
               }
            }

            missing_symbol_bitmaps[sequence_idx].addRange(
               offset + maybe_sequence.size(), genome_length
            );

            if (!positions_with_symbol_missing.empty()) {
               missing_symbol_bitmaps[sequence_idx].addMany(
                  positions_with_symbol_missing.size(), positions_with_symbol_missing.data()
               );
               missing_symbol_bitmaps[sequence_idx].runOptimize();
               positions_with_symbol_missing.clear();
            }
         }
      }
   );
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::optimizeBitmaps() {
   EVOBENCH_SCOPE("SequenceColumnPartition", "optimizeBitmaps");
   std::mutex access_indexing_differences;
   common::parallel_for(
      common::blocked_range(0, positions.size()),
      1,  // positions_per_process, there might be better values
      [&](const auto& local) {
         EVOBENCH_SCOPE_EVERY(100, "SequenceColumnPartition", "optimizeBitmaps-chunk");
         decltype(indexing_differences_to_reference_sequence) local_index_changes{};
         for (auto position_idx = local.begin(); position_idx != local.end(); ++position_idx) {
            auto symbol_changed = positions[position_idx].flipMostNumerousBitmap(sequence_count);
            if (symbol_changed.has_value()) {
               local_index_changes.emplace_back(position_idx, *symbol_changed);
            }
         }

         std::lock_guard<std::mutex> guard(access_indexing_differences);
         for (const auto& element : local_index_changes) {
            indexing_differences_to_reference_sequence.emplace_back(element);
         }
      }
   );
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::flushBuffer() {
   fillIndexes();
   fillNBitmaps();
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeVerticalBitmapsSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      result += position.computeSize();
   }
   return result;
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeHorizontalBitmapsSize() const {
   size_t result = 0;
   for (const auto& bitmap : missing_symbol_bitmaps) {
      result += bitmap.getSizeInBytes(false);
   }
   return result;
}

template <typename SymbolType>
SequenceColumnMetadata<SymbolType>::SequenceColumnMetadata(
   std::string column_name,
   std::vector<typename SymbolType::Symbol>&& reference_sequence
)
    : ColumnMetadata(std::move(column_name)),
      reference_sequence(std::move(reference_sequence)) {}

template class SequenceColumnPartition<Nucleotide>;
template class SequenceColumnPartition<AminoAcid>;
template class SequenceColumnMetadata<Nucleotide>;
template class SequenceColumnMetadata<AminoAcid>;
}  // namespace silo::storage::column
