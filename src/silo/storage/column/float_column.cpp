#include "silo/storage/column/float_column.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition() = default;

const std::vector<double>& FloatColumnPartition::getValues() const {
   return values;
}

void FloatColumnPartition::insert(const std::string& value) {
   double double_value;
   try {
      double_value = value.empty() ? std::nan("") : std::stod(value);
   } catch (std::logic_error& err) {
      SPDLOG_INFO("Double wrongly formatted: '" + value + "'\nInterpreting value as null");
      double_value = std::nan("");
   }
   values.push_back(double_value);
}

FloatColumn::FloatColumn() = default;

FloatColumnPartition& FloatColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
