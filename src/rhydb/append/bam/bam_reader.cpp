#include "rhydb/append/bam/bam_reader.h"

#include <array>
#include <cstring>

#include "rhydb/append/bam/bam_exception.h"

namespace rhydb::append::bam {

namespace {

/// The BAM 4-bit sequence alphabet: nibble value -> IUPAC character. Matches
/// htslib's seq_nt16_str.
constexpr std::string_view SEQ_NIBBLE_TO_CHAR = "=ACMGRSVTWYHKDBN";

/// Little-endian integer assembly from a byte buffer. BAM is defined as
/// little-endian regardless of host byte order, so we assemble explicitly.
uint16_t loadUint16(const unsigned char* bytes) {
   return static_cast<uint16_t>(
      static_cast<uint16_t>(bytes[0]) | static_cast<uint16_t>(bytes[1] << 8)
   );
}

uint32_t loadUint32(const unsigned char* bytes) {
   return static_cast<uint32_t>(bytes[0]) | (static_cast<uint32_t>(bytes[1]) << 8) |
          (static_cast<uint32_t>(bytes[2]) << 16) | (static_cast<uint32_t>(bytes[3]) << 24);
}

int32_t loadInt32(const unsigned char* bytes) {
   return static_cast<int32_t>(loadUint32(bytes));
}

}  // namespace

BamReader::BamReader(std::istream& bam_input)
    : bgzf(bam_input) {
   parseHeader();
}

uint8_t BamReader::readUint8() {
   unsigned char byte = 0;
   bgzf.readExact(reinterpret_cast<char*>(&byte), 1);
   return byte;
}

uint16_t BamReader::readUint16() {
   std::array<unsigned char, 2> bytes{};
   bgzf.readExact(reinterpret_cast<char*>(bytes.data()), bytes.size());
   return loadUint16(bytes.data());
}

uint32_t BamReader::readUint32() {
   std::array<unsigned char, 4> bytes{};
   bgzf.readExact(reinterpret_cast<char*>(bytes.data()), bytes.size());
   return loadUint32(bytes.data());
}

int32_t BamReader::readInt32() {
   return static_cast<int32_t>(readUint32());
}

void BamReader::parseHeader() {
   std::array<char, 4> magic{};
   bgzf.readExact(magic.data(), magic.size());
   if (magic[0] != 'B' || magic[1] != 'A' || magic[2] != 'M' || magic[3] != '\1') {
      throw BamException("input is not a BAM file (bad magic bytes)");
   }

   const uint32_t l_text = readUint32();
   header_text.resize(l_text);
   if (l_text > 0) {
      bgzf.readExact(header_text.data(), l_text);
   }

   const uint32_t n_ref = readUint32();
   reference_list.reserve(n_ref);
   for (uint32_t i = 0; i < n_ref; ++i) {
      const uint32_t l_name = readUint32();
      if (l_name == 0) {
         throw BamException("BAM reference {} has an empty name length", i);
      }
      std::string name(l_name, '\0');
      bgzf.readExact(name.data(), l_name);
      // Names are NUL-terminated in the file; drop the trailing NUL.
      if (!name.empty() && name.back() == '\0') {
         name.pop_back();
      }
      const int32_t l_ref = readInt32();
      reference_list.push_back(BamReference{.name = std::move(name), .length = l_ref});
   }
}

std::optional<BamRecord> BamReader::next() {
   // A clean end of file: the very next byte where a record's block_size would
   // start is absent. A partial read here means the file was truncated.
   std::array<unsigned char, 4> size_bytes{};
   const size_t got = bgzf.read(reinterpret_cast<char*>(size_bytes.data()), size_bytes.size());
   if (got == 0) {
      return std::nullopt;
   }
   if (got < size_bytes.size()) {
      throw BamException("truncated BAM: incomplete record block_size");
   }
   const uint32_t block_size = loadUint32(size_bytes.data());

   // The fixed part of a record (refID .. tlen) is 32 bytes.
   constexpr uint32_t FIXED_RECORD_SIZE = 32;
   if (block_size < FIXED_RECORD_SIZE) {
      throw BamException("BAM record block_size {} is smaller than the fixed header", block_size);
   }

   std::vector<unsigned char> body(block_size);
   bgzf.readExact(reinterpret_cast<char*>(body.data()), block_size);

   BamRecord record;
   record.reference_id = loadInt32(&body[0]);
   record.position = loadInt32(&body[4]);
   const uint8_t l_read_name = body[8];
   record.mapping_quality = body[9];
   // body[10..11] is `bin`, which we do not use.
   const uint16_t n_cigar_op = loadUint16(&body[12]);
   record.flag = loadUint16(&body[14]);
   const int32_t l_seq_signed = loadInt32(&body[16]);
   record.next_reference_id = loadInt32(&body[20]);
   record.next_position = loadInt32(&body[24]);
   record.template_length = loadInt32(&body[28]);

   if (l_read_name == 0) {
      throw BamException("BAM record has an empty read name");
   }
   if (l_seq_signed < 0) {
      throw BamException("BAM record has a negative sequence length {}", l_seq_signed);
   }
   const uint32_t l_seq = static_cast<uint32_t>(l_seq_signed);

   // Compute and bounds-check the layout of the variable-length section.
   const uint32_t seq_bytes = (l_seq + 1) / 2;
   const uint64_t variable_size = static_cast<uint64_t>(l_read_name) +
                                  static_cast<uint64_t>(n_cigar_op) * 4 + seq_bytes + l_seq;
   if (FIXED_RECORD_SIZE + variable_size > block_size) {
      throw BamException(
         "BAM record variable section ({} bytes) overflows block_size {}", variable_size, block_size
      );
   }

   size_t cursor = FIXED_RECORD_SIZE;

   // Read name (drop the trailing NUL).
   record.read_name.assign(reinterpret_cast<char*>(&body[cursor]), l_read_name);
   if (!record.read_name.empty() && record.read_name.back() == '\0') {
      record.read_name.pop_back();
   }
   cursor += l_read_name;

   // CIGAR: each op is (length << 4) | op_code.
   record.cigar.reserve(n_cigar_op);
   for (uint16_t i = 0; i < n_cigar_op; ++i) {
      const uint32_t packed = loadUint32(&body[cursor]);
      cursor += 4;
      const uint32_t op_code = packed & 0xf;
      if (op_code > static_cast<uint32_t>(CigarOp::SEQ_MISMATCH)) {
         throw BamException("BAM record has an unknown CIGAR op code {}", op_code);
      }
      record.cigar.push_back(
         CigarElement{.length = packed >> 4, .op = static_cast<CigarOp>(op_code)}
      );
   }

   // SEQ: 4-bit packed, high nibble first.
   record.sequence.resize(l_seq);
   for (uint32_t i = 0; i < l_seq; ++i) {
      const unsigned char packed = body[cursor + (i / 2)];
      const uint8_t nibble = (i % 2 == 0) ? (packed >> 4) : (packed & 0xf);
      record.sequence[i] = SEQ_NIBBLE_TO_CHAR[nibble];
   }
   cursor += seq_bytes;

   // QUAL: one Phred byte per base; 0xFF throughout means "unavailable".
   if (l_seq == 0 || body[cursor] == 0xFF) {
      record.quality = "*";
   } else {
      record.quality.resize(l_seq);
      for (uint32_t i = 0; i < l_seq; ++i) {
         record.quality[i] = static_cast<char>(body[cursor + i] + 33);
      }
   }

   // Remaining bytes up to block_size are optional tags, which we skip for now.
   return record;
}

}  // namespace rhydb::append::bam
