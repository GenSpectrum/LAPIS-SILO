#include "rhydb/config/clustering_config.h"

#include <cmath>

#include <fmt/format.h>

#include "config/config_exception.h"
#include "config/source/yaml_file.h"

namespace {
using rhydb::config::ConfigKeyPath;
using rhydb::config::YamlFile;

ConfigKeyPath clusteringEnabledOptionKey() {
   return YamlFile::stringToConfigKeyPath("clustering.enabled");
}
ConfigKeyPath clusteringDriverColumnOptionKey() {
   return YamlFile::stringToConfigKeyPath("clustering.driverColumn");
}
ConfigKeyPath clusteringNumBuffersOptionKey() {
   return YamlFile::stringToConfigKeyPath("clustering.numBuffers");
}
ConfigKeyPath clusteringSpanGrowthThresholdOptionKey() {
   return YamlFile::stringToConfigKeyPath("clustering.spanGrowthThreshold");
}
}  // namespace

namespace rhydb::config {

std::vector<ConfigAttributeSpecification> clusteringAttributeSpecifications() {
   const append::ClusteredBufferingOptions defaults;
   return {
      ConfigAttributeSpecification::createWithDefault(
         clusteringEnabledOptionKey(),
         ConfigValue::fromBool(defaults.enabled),
         "Whether ingested rows are grouped into chunks by the covered range of the sequence\n"
         "column given by clustering.driverColumn. This improves compression for data with\n"
         "partial coverage, but no longer stores rows in input order."
      ),
      ConfigAttributeSpecification::createWithoutDefault(
         clusteringDriverColumnOptionKey(),
         ConfigValueType::STRING,
         "The name of the nucleotide or amino-acid sequence column whose covered range decides\n"
         "which chunk a row goes into. Required if clustering.enabled is set."
      ),
      ConfigAttributeSpecification::createWithDefault(
         clusteringNumBuffersOptionKey(),
         ConfigValue::fromUint32(static_cast<uint32_t>(defaults.num_buffers)),
         "The number of chunks that are filled simultaneously during clustered ingestion."
      ),
      ConfigAttributeSpecification::createWithDefault(
         clusteringSpanGrowthThresholdOptionKey(),
         ConfigValue::fromDouble(defaults.span_growth_threshold_fraction),
         "A row joins an existing chunk if this widens the chunk's covered range by at most this\n"
         "fraction of the genome length. Otherwise it starts a new chunk."
      ),
   };
}

void overwriteClusteringOptionsFrom(
   const VerifiedConfigAttributes& config_source,
   append::ClusteredBufferingOptions& options
) {
   if (auto var = config_source.getBool(clusteringEnabledOptionKey())) {
      options.enabled = var.value();
   }
   if (auto var = config_source.getString(clusteringDriverColumnOptionKey())) {
      options.driver_column_name = var.value();
   }
   if (auto var = config_source.getUint32(clusteringNumBuffersOptionKey())) {
      options.num_buffers = var.value();
   }
   if (auto var = config_source.getDouble(clusteringSpanGrowthThresholdOptionKey())) {
      options.span_growth_threshold_fraction = var.value();
   }
}

void validateClusteringOptions(const append::ClusteredBufferingOptions& options) {
   if (!options.enabled) {
      return;
   }
   if (!options.driver_column_name.has_value()) {
      throw ConfigException("'clustering.driverColumn' must be specified if clustering is enabled");
   }
   if (options.num_buffers == 0) {
      throw ConfigException("'clustering.numBuffers' must be at least 1");
   }
   if (
      !std::isfinite(options.span_growth_threshold_fraction) ||
      options.span_growth_threshold_fraction < 0
   ) {
      throw ConfigException(
         fmt::format(
            "'clustering.spanGrowthThreshold' must be a non-negative number, got {}",
            options.span_growth_threshold_fraction
         )
      );
   }
}

}  // namespace rhydb::config
