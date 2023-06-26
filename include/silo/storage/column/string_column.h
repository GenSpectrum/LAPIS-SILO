#ifndef SILO_STRING_COLUMN_H
#define SILO_STRING_COLUMN_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/string.h"
#include "silo/storage/column/column.h"

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class StringColumnPartition : public Column {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& values;
      // clang-format on
   }

   std::vector<common::String<silo::common::STRING_SIZE>> values;
   silo::common::BidirectionalMap<std::string>& lookup;

  public:
   StringColumnPartition(silo::common::BidirectionalMap<std::string>& lookup);

   [[nodiscard]] const std::vector<common::String<silo::common::STRING_SIZE>>& getValues() const;

   void insert(const std::string& value);

   std::optional<common::String<silo::common::STRING_SIZE>> embedString(std::string) const;
};

class StringColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& lookup;
      // clang-format on
      // TODO sync lookups
   }

   std::unique_ptr<silo::common::BidirectionalMap<std::string>> lookup;

  public:
   StringColumn();

   StringColumnPartition createPartition();

   std::optional<common::String<silo::common::STRING_SIZE>> embedString(std::string) const;

   inline std::string lookupValue(common::String<silo::common::STRING_SIZE> string) const {
      return string.toString(*lookup);
   }
};

}  // namespace silo::storage::column

#endif  // SILO_STRING_COLUMN_H
