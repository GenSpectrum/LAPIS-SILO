#include "silo/storage/column/sequence_column.h"

#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/common/table_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/sequence_position.h"
#include "silo/storage/insertion_format_exception.h"
#include "silo/storage/reference_genomes.h"
#include "silo/zstd/zstd_decompressor.h"

namespace silo::storage::column {

template <typename SymbolType>
SequenceColumnPartition<SymbolType>::SequenceColumnPartition(
   SequenceColumnMetadata<SymbolType>* metadata
)
    : metadata(metadata) {
   lazy_buffer.reserve(BUFFER_SIZE);
   genome_length = metadata->reference_sequence.size();
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
void SequenceColumnPartition<SymbolType>::fillIndexes() {
   const size_t sequence_id_base_for_buffer = sequence_count - lazy_buffer.size();
   SymbolMap<SymbolType, std::vector<uint32_t>> ids_per_symbol_for_current_position;
   for (size_t position_idx = 0; position_idx != genome_length; ++position_idx) {
      const size_t number_of_sequences = lazy_buffer.size();
      for (size_t sequence_offset = 0; sequence_offset < number_of_sequences; ++sequence_offset) {
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
         if (symbol != SymbolType::SYMBOL_MISSING && symbol != metadata->reference_sequence.at(position_idx)) {
            ids_per_symbol_for_current_position[*symbol].push_back(
               sequence_id_base_for_buffer + sequence_offset
            );
         }
      }
      addSymbolsToPositions(position_idx, ids_per_symbol_for_current_position, number_of_sequences);
   }
}

namespace {

// TODO tests
std::vector<std::pair<uint16_t, std::vector<uint16_t>>> splitIdsIntoBatches(
   const std::vector<uint32_t>& sorted_ids
) {
   std::vector<std::pair<uint16_t, std::vector<uint16_t>>> ids_in_batches;

   if (sorted_ids.empty()) {
      return ids_in_batches;
   }

   uint16_t current_upper_bits = sorted_ids.front() >> 16;
   ids_in_batches.push_back(std::make_pair<uint16_t, std::vector<uint16_t>>(
      sorted_ids.front() >> 16, std::vector<uint16_t>{}
   ));

   for (uint32_t id : sorted_ids) {
      uint16_t upper_bits = id >> 16;
      uint16_t lower_bits = id & 0xFFFF;

      // If the upper bits changed, start a new batch
      if (upper_bits != current_upper_bits) {
         current_upper_bits = upper_bits;
         ids_in_batches.push_back(std::make_pair(upper_bits, std::vector<uint16_t>{}));
      }

      // Add the lower bits to the current batch
      ids_in_batches.back().second.push_back(lower_bits);
   }

   return ids_in_batches;
}

}  // namespace

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::addSymbolsToPositions(
   uint32_t position_idx,
   SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
   size_t number_of_sequences
) {
   for (const auto& symbol : SymbolType::SYMBOLS) {
      std::vector<std::pair<uint16_t, std::vector<uint16_t>>> ids_in_batches =
         splitIdsIntoBatches(ids_per_symbol_for_current_position[symbol]);
      for (const auto& [upper_bits, lower_bits_vector] : ids_in_batches) {
         SequenceDiffKey key{
            .position = position_idx, .vertical_tile_index = upper_bits, .symbol = symbol
         };
         SequenceDiff* sequence_diff;

         auto iter = vertical_bitmaps.find(key);
         if (iter != vertical_bitmaps.end()) {
            sequence_diff = &iter->second;
         } else {
            // TODO reserve size and/or maybe allocate bitset if card is big
            sequence_diff = &vertical_bitmaps
                                .insert(
                                   {key,
                                    SequenceDiff{
                                       .container = roaring::internal::array_container_create(),
                                       .cardinality = 0,
                                       .typecode = ARRAY_CONTAINER_TYPE
                                    }}
                                )
                                .first->second;
         }

         for (auto id : lower_bits_vector) {
            uint8_t new_container_type;
            auto new_container = roaring::internal::container_add(
               sequence_diff->container, id, sequence_diff->typecode, &new_container_type
            );
            if (new_container != sequence_diff->container) {
               roaring::internal::container_free(sequence_diff->container, sequence_diff->typecode);
               sequence_diff->container = new_container;
               sequence_diff->typecode = new_container_type;
            }
            sequence_diff->cardinality += 1;
         }
      }
      ids_per_symbol_for_current_position[symbol].clear();
   }
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::fillNBitmaps() {
   const size_t sequence_id_base_for_buffer = sequence_count - lazy_buffer.size();

   horizontal_bitmaps.resize(sequence_count);

   const tbb::blocked_range<size_t> range(0, lazy_buffer.size());
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      std::vector<uint32_t> positions_with_symbol_missing;
      for (size_t sequence_offset_in_buffer = local.begin();
           sequence_offset_in_buffer != local.end();
           ++sequence_offset_in_buffer) {
         const auto& [is_valid, maybe_sequence, offset] = lazy_buffer[sequence_offset_in_buffer];

         const size_t sequence_idx = sequence_id_base_for_buffer + sequence_offset_in_buffer;

         if (!is_valid) {
            horizontal_bitmaps[sequence_idx].addRange(0, genome_length);
            horizontal_bitmaps[sequence_idx].runOptimize();
            continue;
         }

         horizontal_bitmaps[sequence_idx].addRange(0, offset);

         for (size_t position_idx = 0; position_idx < maybe_sequence.size(); ++position_idx) {
            const char character = maybe_sequence[position_idx];
            const auto symbol = SymbolType::charToSymbol(character);
            if (symbol == SymbolType::SYMBOL_MISSING) {
               positions_with_symbol_missing.push_back(position_idx + offset);
            }
         }

         horizontal_bitmaps[sequence_idx].addRange(offset + maybe_sequence.size(), genome_length);

         if (!positions_with_symbol_missing.empty()) {
            horizontal_bitmaps[sequence_idx].addMany(
               positions_with_symbol_missing.size(), positions_with_symbol_missing.data()
            );
            horizontal_bitmaps[sequence_idx].runOptimize();
            positions_with_symbol_missing.clear();
         }
      }
   });
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::optimizeBitmaps() {
   // TODO
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::flushBuffer() {
   fillIndexes();
   fillNBitmaps();
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeVerticalBitmapsSize() const {
   size_t result = 0;
   for (const auto& [_, sequence_diff] : vertical_bitmaps) {
      result += roaring::internal::container_size_in_bytes(
         sequence_diff.container, sequence_diff.typecode
      );
   }
   return result;
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeHorizontalBitmapsSize() const {
   size_t result = 0;
   for (const auto& bitmap : horizontal_bitmaps) {
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
