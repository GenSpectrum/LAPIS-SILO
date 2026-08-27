#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace rhydb::append::bam {

/// BAM CIGAR operations, in the integer encoding used by the BAM binary format
/// (the low 4 bits of each packed cigar element). The comment shows the SAM
/// textual spelling and what the op consumes.
enum class CigarOp : uint8_t {
   MATCH = 0,         // M  consumes query + reference (match or mismatch)
   INSERTION = 1,     // I  consumes query only
   DELETION = 2,      // D  consumes reference only
   SKIP = 3,          // N  consumes reference only (e.g. an intron)
   SOFT_CLIP = 4,     // S  consumes query only
   HARD_CLIP = 5,     // H  consumes neither
   PADDING = 6,       // P  consumes neither
   SEQ_MATCH = 7,     // =  consumes query + reference
   SEQ_MISMATCH = 8,  // X  consumes query + reference
};

struct CigarElement {
   uint32_t length;
   CigarOp op;
};

/// A read projected onto reference coordinates, in exactly the shape the
/// nucleotide-sequence ingest expects: a contiguous aligned segment starting at
/// `offset` (a 0-based reference position) plus a list of insertions in the
/// "position:bases" form.
struct AlignedRead {
   /// 0-based reference position of the first base of `aligned_sequence`.
   uint32_t offset;
   /// One character per reference position spanned by the alignment: the read's
   /// base for M/=/X, '-' (GAP) for a deletion (D), 'N' for a reference skip (N).
   std::string aligned_sequence;
   /// Insertions relative to the reference, each formatted "<0-based ref pos>:<bases>".
   std::vector<std::string> insertions;
};

/// Project a mapped read onto reference coordinates.
///
/// `reference_start` is the read's 0-based leftmost mapped reference position
/// (BAM `pos`). `read_bases` is the read-oriented sequence (the decoded SEQ);
/// its length must equal the number of query-consuming CIGAR bases (M/I/S/=/X).
/// Soft clips are dropped and do not shift `offset`; hard clips and padding are
/// ignored.
///
/// Returns an error string when the CIGAR consumes a different number of query
/// bases than `read_bases` provides.
[[nodiscard]] std::expected<AlignedRead, std::string> projectReadOntoReference(
   uint32_t reference_start,
   std::string_view read_bases,
   const std::vector<CigarElement>& cigar
);

}  // namespace rhydb::append::bam
