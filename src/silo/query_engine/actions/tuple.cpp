#include "silo/query_engine/actions/tuple.h"

#include "silo/query_engine/actions/action.h"

using namespace silo::query_engine::actions;

using json_value_type = std::optional<std::variant<std::string, int32_t, double>>;

namespace {
using silo::common::Date;
using silo::common::String;
using silo::common::STRING_SIZE;
using silo::config::ColumnType;

void assignTupleField(
   char** data_pointer,
   uint32_t sequence_id,
   const silo::storage::ColumnMetadata& metadata,
   const silo::storage::ColumnPartitionGroup& columns
) {
   if (metadata.type == ColumnType::DATE) {
      const Date value = columns.date_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<Date*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::INT) {
      const int32_t value = columns.int_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<int32_t*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::FLOAT) {
      const double value = columns.float_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<double*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::STRING) {
      const String<STRING_SIZE> value =
         columns.string_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<String<STRING_SIZE>*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::INDEXED_PANGOLINEAGE) {
      const silo::Idx value =
         columns.pango_lineage_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<silo::Idx*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::INDEXED_STRING) {
      const silo::Idx value =
         columns.indexed_string_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<silo::Idx*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else if (metadata.type == ColumnType::INSERTION) {
      const silo::Idx value = columns.insertion_columns.at(metadata.name).getValues()[sequence_id];
      *reinterpret_cast<silo::Idx*>(*data_pointer) = value;
      *data_pointer += sizeof(decltype(value));
   } else {
      throw std::runtime_error("Unchecked column type of column " + metadata.name);
   }
}

json_value_type tupleFieldToValueType(
   const char** data_pointer,
   const silo::storage::ColumnMetadata& metadata,
   const silo::storage::ColumnPartitionGroup& columns
) {
   if (metadata.type == ColumnType::DATE) {
      const Date value = *reinterpret_cast<const Date*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      return silo::common::dateToString(value);
   } else if (metadata.type == ColumnType::INT) {
      const int32_t value = *reinterpret_cast<const int32_t*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      if (value == INT32_MIN) {
         return std::nullopt;
      } else {
         return value;
      }
   } else if (metadata.type == ColumnType::FLOAT) {
      const double value = *reinterpret_cast<const double*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      if (std::isnan(value)) {
         return std::nullopt;
      } else {
         return value;
      }
   } else if (metadata.type == ColumnType::STRING) {
      const String<STRING_SIZE> value =
         *reinterpret_cast<const String<STRING_SIZE>*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      std::string string_value = columns.string_columns.at(metadata.name).lookupValue(value);
      if (string_value.empty()) {
         return std::nullopt;
      } else {
         return std::move(string_value);
      }
   } else if (metadata.type == ColumnType::INDEXED_PANGOLINEAGE) {
      const silo::Idx value = *reinterpret_cast<const silo::Idx*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      std::string string_value =
         columns.pango_lineage_columns.at(metadata.name).lookupValue(value).value;
      if (string_value.empty()) {
         return std::nullopt;
      } else {
         return std::move(string_value);
      }
   } else if (metadata.type == ColumnType::INDEXED_STRING) {
      const silo::Idx value = *reinterpret_cast<const silo::Idx*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      std::string string_value =
         columns.indexed_string_columns.at(metadata.name).lookupValue(value);
      if (string_value.empty()) {
         return std::nullopt;
      } else {
         return std::move(string_value);
      }
   } else if (metadata.type == ColumnType::INSERTION) {
      const silo::Idx value = *reinterpret_cast<const silo::Idx*>(*data_pointer);
      *data_pointer += sizeof(decltype(value));
      std::string string_value = columns.insertion_columns.at(metadata.name).lookupValue(value);
      if (string_value.empty()) {
         return std::nullopt;
      } else {
         return std::move(string_value);
      }
   } else {
      throw std::runtime_error("Unchecked column type of column " + metadata.name);
   }
}

std::strong_ordering compareDouble(double value1, double value2) {
   std::partial_ordering compare = value1 <=> value2;
   if (compare == std::partial_ordering::less) {
      return std::strong_ordering::less;
   }
   if (compare == std::partial_ordering::equivalent) {
      return std::strong_ordering::equal;
   }
   if (compare == std::partial_ordering::greater) {
      return std::strong_ordering::greater;
   }
   if (std::isnan(value2)) {
      if (std::isnan(value1)) {
         return std::strong_ordering::equal;
      } else {
         return std::strong_ordering::less;
      }
   } else {
      return std::strong_ordering::greater;
   }
}

std::strong_ordering compareString(const std::string& value1, const std::string& value2) {
   int cmp = value1.compare(value2);
   if (cmp < 0) {
      return std::strong_ordering::less;
   }
   if (cmp > 0) {
      return std::strong_ordering::greater;
   }
   return std::strong_ordering::equal;
}

std::strong_ordering compareTupleFields(
   const char** data_pointer1,
   const char** data_pointer2,
   const silo::storage::ColumnMetadata& metadata,
   const silo::storage::ColumnPartitionGroup& columns
) {
   if (metadata.type == ColumnType::DATE) {
      const Date value1 = *reinterpret_cast<const Date*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      const Date value2 = *reinterpret_cast<const Date*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      return value1 <=> value2;
   } else if (metadata.type == ColumnType::INT) {
      const int32_t value1 = *reinterpret_cast<const int32_t*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      const int32_t value2 = *reinterpret_cast<const int32_t*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      return value1 <=> value2;
   } else if (metadata.type == ColumnType::FLOAT) {
      const double value1 = *reinterpret_cast<const double*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      const double value2 = *reinterpret_cast<const double*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      return compareDouble(value1, value2);
   } else if (metadata.type == ColumnType::STRING) {
      const String<STRING_SIZE> value1 =
         *reinterpret_cast<const String<STRING_SIZE>*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      const String<STRING_SIZE> value2 =
         *reinterpret_cast<const String<STRING_SIZE>*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));

      auto fast_compare = value1.fastCompare(value2);
      if (fast_compare) {
         return fast_compare.value();
      }
      std::string string_value1 = columns.string_columns.at(metadata.name).lookupValue(value1);
      std::string string_value2 = columns.string_columns.at(metadata.name).lookupValue(value2);
      return compareString(string_value1, string_value2);
   } else if (metadata.type == ColumnType::INDEXED_PANGOLINEAGE) {
      const silo::Idx value1 = *reinterpret_cast<const silo::Idx*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      std::string string_value1 =
         columns.pango_lineage_columns.at(metadata.name).lookupValue(value1).value;
      const silo::Idx value2 = *reinterpret_cast<const silo::Idx*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      std::string string_value2 =
         columns.pango_lineage_columns.at(metadata.name).lookupValue(value2).value;
      return compareString(string_value1, string_value2);
   } else if (metadata.type == ColumnType::INDEXED_STRING) {
      const silo::Idx value1 = *reinterpret_cast<const silo::Idx*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      std::string string_value1 =
         columns.indexed_string_columns.at(metadata.name).lookupValue(value1);
      const silo::Idx value2 = *reinterpret_cast<const silo::Idx*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      std::string string_value2 =
         columns.indexed_string_columns.at(metadata.name).lookupValue(value2);
      return compareString(string_value1, string_value2);
   } else if (metadata.type == ColumnType::INSERTION) {
      const silo::Idx value1 = *reinterpret_cast<const silo::Idx*>(*data_pointer1);
      *data_pointer1 += sizeof(decltype(value1));
      std::string string_value1 = columns.insertion_columns.at(metadata.name).lookupValue(value1);
      const silo::Idx value2 = *reinterpret_cast<const silo::Idx*>(*data_pointer2);
      *data_pointer2 += sizeof(decltype(value2));
      std::string string_value2 = columns.insertion_columns.at(metadata.name).lookupValue(value2);
      return compareString(string_value1, string_value2);
   } else {
      throw std::runtime_error("Unchecked column type of column " + metadata.name);
   }
}

size_t getColumnSize(const silo::storage::ColumnMetadata& metadata) {
   if (metadata.type == silo::config::ColumnType::STRING) {
      return sizeof(silo::common::String<silo::common::STRING_SIZE>);
   } else if (metadata.type == silo::config::ColumnType::FLOAT) {
      return sizeof(double);
   } else if (metadata.type == silo::config::ColumnType::INT) {
      return sizeof(int32_t);
   } else if (metadata.type == silo::config::ColumnType::DATE) {
      return sizeof(silo::common::Date);
   } else {
      return sizeof(silo::Idx);
   }
}

}  // namespace

size_t silo::query_engine::actions::getTupleSize(
   const std::vector<silo::storage::ColumnMetadata>& metadata_list
) {
   size_t size = 0;
   for (const auto& metadata : metadata_list) {
      size += getColumnSize(metadata);
   }
   return size;
}

Tuple::Tuple(
   uint32_t sequence_id,
   const silo::storage::ColumnPartitionGroup* columns,
   size_t tuple_size
)
    : columns(columns) {
   data.resize(tuple_size);
   char* data_pointer = data.data();
   for (const auto& metadata : columns->metadata) {
      assignTupleField(&data_pointer, sequence_id, metadata, *columns);
   }
}

Tuple::Tuple(const Tuple& other)
    : columns(other.columns),
      data(other.data) {}

Tuple::Tuple(Tuple&& other)
    : columns(other.columns),
      data(std::move(other.data)) {}

Tuple& Tuple::operator=(const Tuple& other) {
   this->columns = other.columns;
   this->data = other.data;
   return *this;
};

Tuple& Tuple::operator=(Tuple&& other) {
   this->columns = other.columns;
   this->data = std::move(other.data);
   return *this;
};

std::map<std::string, json_value_type> Tuple::getFields() const {
   std::map<std::string, json_value_type> fields;
   const char* data_pointer = data.data();
   for (const auto& metadata : columns->metadata) {
      fields[metadata.name] = tupleFieldToValueType(&data_pointer, metadata, *columns);
   }
   return fields;
}

std::vector<TupleFieldComparator> Tuple::getCompareFields(
   const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
   const std::vector<OrderByField>& order_by_fields
) {
   std::vector<TupleFieldComparator> tuple_field_comparators;
   tuple_field_comparators.resize(order_by_fields.size());
   size_t offset = 0;
   for (const auto& metadata : columns_metadata) {
      auto it = std::find_if(
         order_by_fields.begin(),
         order_by_fields.end(),
         [&](const auto& order_by_field) { return metadata.name == order_by_field.name; }
      );
      if (it != order_by_fields.end()) {
         const size_t index = std::distance(order_by_fields.begin(), it);
         tuple_field_comparators[index] = TupleFieldComparator{offset, metadata, it->ascending};
      }
      offset += getColumnSize(metadata);
   }
   return tuple_field_comparators;
}

Tuple::Comparator Tuple::getComparator(
   const std::vector<silo::storage::ColumnMetadata>& columns_metadata,
   const std::vector<OrderByField>& order_by_fields
) {
   auto tuple_field_comparators =
      actions::Tuple::getCompareFields(columns_metadata, order_by_fields);
   return [tuple_field_comparators](const Tuple& tuple1, const Tuple& tuple2) {
      return tuple1.compareLess(tuple2, tuple_field_comparators);
   };
}

bool Tuple::compareLess(const Tuple& other, const std::vector<TupleFieldComparator>& fields) const {
   for (const auto& field : fields) {
      auto data_pointer1 = (this->data.data() + field.offset);
      auto data_pointer2 = (other.data.data() + field.offset);
      std::strong_ordering compare =
         compareTupleFields(&data_pointer1, &data_pointer2, field.type, *columns);
      if (compare != std::strong_ordering::equal) {
         if (compare == std::strong_ordering::less) {
            return field.ascending;
         } else {
            return !field.ascending;
         }
      }
   }
   return false;
}

bool Tuple::operator==(const Tuple& other) const {
   return this->data == other.data;
}
bool Tuple::operator!=(const Tuple& other) const {
   return !(*this == other);
}

bool Tuple::operator<(const Tuple& other) const {
   const char* data_pointer1 = data.data();
   const char* data_pointer2 = other.data.data();
   for (const auto& metadata : columns->metadata) {
      std::strong_ordering compare =
         compareTupleFields(&data_pointer1, &data_pointer2, metadata, *columns);
      if (compare != std::strong_ordering::equal) {
         return compare == std::strong_ordering::less;
      }
   }
   return false;
}

bool Tuple::operator>(const Tuple& other) const {
   return (other < *this);
}
bool Tuple::operator<=(const Tuple& other) const {
   return !(other < *this);
}
bool Tuple::operator>=(const Tuple& other) const {
   return !(*this < other);
}
