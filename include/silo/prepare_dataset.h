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

pango_descriptor_t build_pango_defs(const alias_key_t& alias_key, std::istream& meta_in);

void save_pango_defs(const pango_descriptor_t& pd, std::ostream& out);

pango_descriptor_t load_pango_defs(std::istream& in);

partitioning_descriptor_t build_partitioning_descriptor(pango_descriptor_t pango_defs, architecture_type arch);

void save_partitioning_descriptor(const partitioning_descriptor_t& pd, std::ostream& out);

partitioning_descriptor_t load_partitioning_descriptor(std::istream& in);

void partition_sequences(const partitioning_descriptor_t& pd, std::istream& meta_in, std::istream& sequence_in,
                         const std::string& output_prefix, const alias_key_t& alias_key);

void sort_chunks(const partitioning_descriptor_t& pd, const std::string& output_prefix);

} // namespace silo

#endif //SILO_PREPARE_DATASET_H
