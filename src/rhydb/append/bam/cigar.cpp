#include "rhydb/append/bam/cigar.h"

#include <fmt/format.h>

namespace rhydb::append::bam {

namespace {

bool consumesQuery(CigarOp op) {
   switch (op) {
      case CigarOp::MATCH:
      case CigarOp::INSERTION:
      case CigarOp::SOFT_CLIP:
      case CigarOp::SEQ_MATCH:
      case CigarOp::SEQ_MISMATCH:
         return true;
      case CigarOp::DELETION:
      case CigarOp::SKIP:
      case CigarOp::HARD_CLIP:
      case CigarOp::PADDING:
         return false;
   }
   return false;
}

}  // namespace

std::expected<AlignedRead, std::string> projectReadOntoReference(
   uint32_t reference_start,
   std::string_view read_bases,
   const std::vector<CigarElement>& cigar
) {
   // The CIGAR's query-consuming ops must account for exactly the read sequence,
   // otherwise the record is malformed and slicing read_bases would be wrong.
   size_t query_consumed = 0;
   for (const auto& element : cigar) {
      if (consumesQuery(element.op)) {
         query_consumed += element.length;
      }
   }
   if (query_consumed != read_bases.size()) {
      return std::unexpected(fmt::format(
         "CIGAR consumes {} query bases but the read sequence has {} bases",
         query_consumed,
         read_bases.size()
      ));
   }

   AlignedRead result;
   result.offset = reference_start;

   // `reference_cursor` tracks the absolute 0-based reference position; `read_cursor`
   // indexes into read_bases. The aligned segment is grown one op at a time so it
   // stays contiguous from `offset`.
   uint32_t reference_cursor = reference_start;
   size_t read_cursor = 0;
   for (const auto& element : cigar) {
      const uint32_t length = element.length;
      switch (element.op) {
         case CigarOp::MATCH:
         case CigarOp::SEQ_MATCH:
         case CigarOp::SEQ_MISMATCH:
            result.aligned_sequence.append(read_bases.data() + read_cursor, length);
            read_cursor += length;
            reference_cursor += length;
            break;
         case CigarOp::INSERTION:
            result.insertions.push_back(fmt::format(
               "{}:{}", reference_cursor, std::string_view{read_bases.data() + read_cursor, length}
            ));
            read_cursor += length;
            break;
         case CigarOp::DELETION:
            result.aligned_sequence.append(length, '-');
            reference_cursor += length;
            break;
         case CigarOp::SKIP:
            result.aligned_sequence.append(length, 'N');
            reference_cursor += length;
            break;
         case CigarOp::SOFT_CLIP:
            read_cursor += length;
            break;
         case CigarOp::HARD_CLIP:
         case CigarOp::PADDING:
            break;
      }
   }

   return result;
}

}  // namespace rhydb::append::bam
