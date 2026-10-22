#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "config/config_specification.h"
#include "config/verified_config_attributes.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/common/json_type_definitions.h"

namespace rhydb::append {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
   ClusteredBufferingOptions,
   enabled,
   driver_column_name,
   num_buffers,
   span_growth_threshold_fraction
)

}  // namespace rhydb::append

/// The `clustering.*` options shared by every program that ingests data (preprocessing, append).
/// They configure append::ClusteredBufferingOptions and default to its values.
namespace rhydb::config {

std::vector<ConfigAttributeSpecification> clusteringAttributeSpecifications();

void overwriteClusteringOptionsFrom(
   const VerifiedConfigAttributes& config_source,
   append::ClusteredBufferingOptions& options
);

/// Throws a ConfigException if the options are enabled but not usable.
void validateClusteringOptions(const append::ClusteredBufferingOptions& options);

}  // namespace rhydb::config
