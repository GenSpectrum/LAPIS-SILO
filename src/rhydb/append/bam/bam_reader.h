#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <string>
#include <vector>

#include "rhydb/append/bam/bgzf_reader.h"
#include "rhydb/append/bam/cigar.h"

namespace rhydb::append::bam {

/// One entry of the BAM header's reference dictionary (an @SQ line).
struct BamReference {
   std::string name;
   int32_t length;
};

/// A single decoded BAM alignment record. Fields mirror the BAM binary layout;
/// SEQ is already nibble-decoded to an IUPAC string and QUAL is rendered Phred+33
/// (or "*" when absent). Optional tags are not decoded in this first version.
struct BamRecord {
   int32_t reference_id;  // index into the reference dictionary; -1 = unmapped
   int32_t position;      // 0-based leftmost mapped position; -1 = unavailable
   uint8_t mapping_quality;
   uint16_t flag;
   std::string read_name;
   std::vector<CigarElement> cigar;
   std::string sequence;  // decoded read bases; empty when SEQ is absent (l_seq == 0)
   std::string quality;   // Phred+33 ASCII, or "*" when quality is unavailable
   int32_t next_reference_id;
   int32_t next_position;
   int32_t template_length;

   /// SAM flag bits used for the default record filtering.
   [[nodiscard]] bool isUnmapped() const { return (flag & 0x4) != 0; }
   [[nodiscard]] bool isSecondary() const { return (flag & 0x100) != 0; }
   [[nodiscard]] bool isSupplementary() const { return (flag & 0x800) != 0; }
};

/// Reads a BAM file: decompresses the BGZF container, parses the header and its
/// reference dictionary up front, then yields alignment records one at a time.
class BamReader {
  public:
   explicit BamReader(std::istream& bam_input);

   [[nodiscard]] const std::vector<BamReference>& references() const { return reference_list; }
   [[nodiscard]] const std::string& headerText() const { return header_text; }

   /// Parse and return the next alignment record, or nullopt at end of file.
   /// Throws BamException on malformed input.
   std::optional<BamRecord> next();

  private:
   uint8_t readUint8();
   uint16_t readUint16();
   int32_t readInt32();
   uint32_t readUint32();

   void parseHeader();

   BgzfReader bgzf;
   std::string header_text;
   std::vector<BamReference> reference_list;
};

}  // namespace rhydb::append::bam
