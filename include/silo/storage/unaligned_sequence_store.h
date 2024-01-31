#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
class ZstdFastaTableReader;

class UnalignedSequenceStorePartition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      archive & sequence_count;
   }

  public:
   std::filesystem::path file_name;
   std::string& compression_dictionary;
   uint32_t sequence_count = 0;

   explicit UnalignedSequenceStorePartition(
      std::filesystem::path file_name,
      std::string& compression_dictionary
   );

   size_t fill(silo::ZstdFastaTableReader& input);
};

class UnalignedSequenceStore {
  public:
   std::deque<UnalignedSequenceStorePartition> partitions;
   std::filesystem::path folder_path;
   std::string compression_dictionary;

   void saveFolder(const std::filesystem::path& save_location) const;

   explicit UnalignedSequenceStore(
      std::filesystem::path folder_path,
      std::string&& compression_dictionary
   );

   UnalignedSequenceStorePartition& createPartition();
};

}  // namespace silo
