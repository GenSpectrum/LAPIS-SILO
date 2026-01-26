#include "silo/storage/column/sequence_column.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>
#include <roaring/roaring.hh>

#include "silo/append/append_exception.h"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/storage/insertion_format_exception.h"

namespace silo::storage::column {

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
               "parsable as an integer, instead got: '{}'",
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
SequenceColumnPartition<SymbolType>::SequenceColumnPartition(
   SequenceColumnMetadata<SymbolType>* metadata
)
    : metadata(metadata),
      genome_length(metadata->reference_sequence.size()),
      local_reference_sequence_string(SymbolType::sequenceToString(metadata->reference_sequence)),
      horizontal_coverage_index() {
   mutation_buffer.resize(genome_length);
   SILO_ASSERT_GT(genome_length, 0);
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::append(
   std::string_view sequence,
   uint32_t offset,
   std::vector<std::string> insertions
) {
   uint32_t sequence_idx = sequence_count;
   sequence_count++;

   horizontal_coverage_index.insertSequenceCoverage<SymbolType>(sequence, offset);

   if (sequence.size() + offset > genome_length) {
      throw append::AppendException(
         "the sequence '{}' which was inserted with an offset {} is larger than the length of "
         "the reference genome: {}",
         sequence,
         offset,
         genome_length
      );
   }
   for (size_t char_in_sequence = 0; char_in_sequence != sequence.size(); ++char_in_sequence) {
      size_t position_idx = char_in_sequence + offset;

      if (sequence.at(char_in_sequence) == local_reference_sequence_string.at(position_idx)) {
         continue;
      }

      const char character = sequence.at(char_in_sequence);
      const auto symbol = SymbolType::charToSymbol(character);
      if (!symbol.has_value()) {
         throw append::AppendException(
            "illegal character '{}' at position {} contained in sequence with index {} in "
            "the current input",
            character,
            position_idx,
            sequence_idx
         );
      }
      if (symbol != SymbolType::SYMBOL_MISSING) {
         mutation_buffer.at(position_idx)[*symbol].push_back(sequence_idx);
      }
   }

   for (auto& insertion_and_position : insertions) {
      auto [position, insertion] = parseInsertion<SymbolType>(insertion_and_position);
      insertion_index.addLazily(position, insertion, sequence_idx);
   }

   // Flush buffer every 1 << 16 = 65536 sequences
   if ((sequence_count % (1 << 16)) == 0) {
      flushBuffer();
   }
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::appendNull() {
   sequence_count += 1;
   horizontal_coverage_index.insertNullSequence();
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::finalize() {
   flushBuffer();

   SPDLOG_DEBUG("Building insertion index");
   insertion_index.buildIndex();

   const SequenceColumnInfo info_after_filling = calculateInfo();

   SPDLOG_DEBUG("Adapting local reference");
   constexpr size_t BATCH_SIZE = 1024;
   for (uint32_t position_range_start = 0; position_range_start < genome_length;
        position_range_start += BATCH_SIZE) {
      auto coverage_bitmaps =
         horizontal_coverage_index.getCoverageBitmapForPositions<BATCH_SIZE>(position_range_start);

      size_t range_size = std::min(BATCH_SIZE, genome_length - position_range_start);
      for (size_t position_idx = position_range_start;
           position_idx < position_range_start + range_size;
           ++position_idx) {
         const roaring::Roaring& coverage_bitmap =
            coverage_bitmaps[position_idx - position_range_start];
         auto new_reference_symbol = vertical_sequence_index.adaptLocalReference(
            coverage_bitmap,
            position_idx,
            SymbolType::charToSymbol(local_reference_sequence_string.at(position_idx)).value()
         );
         if (new_reference_symbol.has_value()) {
            SPDLOG_DEBUG(
               "At position {} adapted local reference symbol to '{}'",
               position_idx,
               SymbolType::symbolToChar(new_reference_symbol.value())
            );
            local_reference_sequence_string.at(position_idx) =
               SymbolType::symbolToChar(new_reference_symbol.value());
         }
      }
   }

   const SequenceColumnInfo info_after_adaption = calculateInfo();

   SPDLOG_DEBUG("Optimizing bitmaps");

   optimizeBitmaps();

   const SequenceColumnInfo info_after_optimisation = calculateInfo();

   SPDLOG_DEBUG(
      "Sequence store partition info after filling it: {}, after local reference adaption: {}, and "
      "after optimising: {}",
      info_after_filling,
      info_after_adaption,
      info_after_optimisation
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
   for (size_t position_idx = 0; position_idx != genome_length; ++position_idx) {
      vertical_sequence_index.addSymbolsToPositions(position_idx, mutation_buffer.at(position_idx));
   }
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::optimizeBitmaps() {
   for (auto& [sequence_diff_key, sequence_diff] : vertical_sequence_index.vertical_bitmaps) {
      uint8_t new_container_type;
      auto new_container = roaring::internal::convert_run_optimize(
         sequence_diff.container, sequence_diff.typecode, &new_container_type
      );
      if (new_container != sequence_diff.container) {
         sequence_diff.container = new_container;
         sequence_diff.typecode = new_container_type;
      }
      roaring::internal::container_shrink_to_fit(sequence_diff.container, sequence_diff.typecode);
   }
}

template <typename SymbolType>
void SequenceColumnPartition<SymbolType>::flushBuffer() {
   fillIndexes();
   for (auto& position : mutation_buffer) {
      for (auto symbol : SymbolType::SYMBOLS) {
         position[symbol].clear();
      }
   }
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeVerticalBitmapsSize() const {
   size_t result = 0;
   for (const auto& [_pos, sequence_diff] : vertical_sequence_index.vertical_bitmaps) {
      result += roaring::internal::container_size_in_bytes(
         sequence_diff.container, sequence_diff.typecode
      );
   }
   return result;
}

template <typename SymbolType>
size_t SequenceColumnPartition<SymbolType>::computeHorizontalBitmapsSize() const {
   size_t result = 0;
   for (const auto& [_pos, bitmap] : horizontal_coverage_index.horizontal_bitmaps) {
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
