#pragma once

#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>

namespace silo::storage::column {

class ColumnMetadata {
  public:
   std::string column_name;

   explicit ColumnMetadata(std::string column_name)
       : column_name(column_name) {}

   virtual ~ColumnMetadata() = default;
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::ColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::ColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
}
}  // namespace boost::serialization
BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<silo::storage::column::ColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::ColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   ar & column_name;
   object = std::make_shared<silo::storage::column::ColumnMetadata>(std::move(column_name));
}
}  // namespace boost::serialization
