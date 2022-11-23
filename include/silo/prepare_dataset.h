//
// Created by Alexander Taepper on 16.11.22.
//

#ifndef SILO_PREPARE_DATASET_H
#define SILO_PREPARE_DATASET_H

#include <silo/database.h>
#include <silo/silo.h>

namespace silo {

void prune_sequences(std::istream& meta_in, std::istream& sequences_in, std::ostream& sequences_out);

void prune_meta(std::istream& meta_in, std::istream& sequences_in, std::ostream& meta_out);

pango_descriptor_t build_pango_defs(const std::unordered_map<std::string, std::string>& alias_key, std::istream& meta_in);

partitioning_descriptor_t build_partitioning_descriptor(pango_descriptor_t pango_defs, architecture_type arch);

void partition_sequences(const partitioning_descriptor_t& pd, std::istream& meta_in, std::istream& sequence_in,
                         const std::string& output_prefix, const std::unordered_map<std::string, std::string>& alias_key);

void sort_chunks(const partitioning_descriptor_t& pd, const std::string& output_prefix);

} // namespace silo

#endif //SILO_PREPARE_DATASET_H
