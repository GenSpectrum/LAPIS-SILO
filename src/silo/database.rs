use std::collections::{HashMap, BTreeMap};
use std::sync::atomic::{AtomicU32, AtomicU64, Ordering};
use std::sync::Mutex;
use std::sync::Arc;
use std::fs::File;
use std::io::{self, Read, Write};
use std::path::Path;
use std::sync::mpsc::channel;
use std::thread;

use roaring::RoaringBitmap;
use serde::{Deserialize, Serialize};
use serde_json::json;
use tbb::parallel_for_each;

use crate::common::block_timer::BlockTimer;
use crate::common::data_version::DataVersion;
use crate::common::fasta_reader::FastaReader;
use crate::common::format_number::format_number;
use crate::common::nucleotide_symbols::Nucleotide;
use crate::config::database_config::DatabaseConfig;
use crate::config::preprocessing_config::PreprocessingConfig;
use crate::database_info::DatabaseInfo;
use crate::persistence::exception::{LoadDatabaseException, SaveDatabaseException};
use crate::preprocessing::metadata_info::MetadataInfo;
use crate::preprocessing::preprocessing_exception::PreprocessingException;
use crate::query_engine::query_engine::QueryEngine;
use crate::query_engine::query_result::QueryResult;
use crate::roaring::roaring_serialize::RoaringSerialize;
use crate::storage::column::date_column::DateColumn;
use crate::storage::column::float_column::FloatColumn;
use crate::storage::column::indexed_string_column::IndexedStringColumn;
use crate::storage::column::int_column::IntColumn;
use crate::storage::column::pango_lineage_column::PangoLineageColumn;
use crate::storage::column::string_column::StringColumn;
use crate::storage::column_group::ColumnGroup;
use crate::storage::database_partition::DatabasePartition;
use crate::storage::pango_lineage_alias::PangoLineageAlias;
use crate::storage::reference_genomes::ReferenceGenomes;
use crate::storage::sequence_store::{SequenceStore, Nucleotide, AminoAcid};
use crate::storage::serialize_optional::SerializeOptional;
use crate::storage::unaligned_sequence_store::UnalignedSequenceStore;
use crate::zstdfasta::zstd_decompressor::ZstdDecompressor;
use crate::zstdfasta::zstdfasta_table::ZstdFastaTable;
use crate::zstdfasta::zstdfasta_table_reader::ZstdFastaTableReader;

pub struct Database {
    database_config: DatabaseConfig,
    partitions: Vec<DatabasePartition>,
    nuc_sequences: HashMap<String, SequenceStore<Nucleotide>>,
    aa_sequences: HashMap<String, SequenceStore<AminoAcid>>,
    alias_key: PangoLineageAlias,
    columns: ColumnGroup,
    data_version: DataVersion,
    intermediate_results_directory: String,
}

impl Database {
    pub fn get_sequence_names<T>(&self) -> Vec<String> {
        if std::any::TypeId::of::<T>() == std::any::TypeId::of::<Nucleotide>() {
            self.nuc_sequences.keys().cloned().collect()
        } else if std::any::TypeId::of::<T>() == std::any::TypeId::of::<AminoAcid>() {
            self.aa_sequences.keys().cloned().collect()
        } else {
            Vec::new()
        }
    }
}

impl Database {
    pub fn get_default_sequence_name<T>(&self) -> Option<String> {
        if std::any::TypeId::of::<T>() == std::any::TypeId::of::<Nucleotide>() {
            self.database_config.default_nucleotide_sequence.clone()
        } else if std::any::TypeId::of::<T>() == std::any::TypeId::of::<AminoAcid>() {
            self.database_config.default_amino_acid_sequence.clone()
        } else {
            None
        }
    }
}

impl Database {
    pub fn new(database_config: DatabaseConfig) -> Self {
        Self {
            database_config,
            partitions: Vec::new(),
            nuc_sequences: HashMap::new(),
            aa_sequences: HashMap::new(),
            alias_key: PangoLineageAlias::new(),
            columns: ColumnGroup::new(),
            data_version: DataVersion::new(),
            intermediate_results_directory: String::new(),
        }
    }

    pub fn validate(&self) {
        for partition in &self.partitions {
            partition.validate();
        }
    }

    pub fn get_database_info(&self) -> DatabaseInfo {
        let sequence_count = AtomicU32::new(0);
        let total_size = AtomicU64::new(0);
        let nucleotide_symbol_n_bitmaps_size = AtomicU64::new(0);

        parallel_for_each(&self.partitions, |database_partition| {
            let mut local_total_size = 0;
            let mut local_nucleotide_symbol_n_bitmaps_size = 0;
            for seq_store in database_partition.nuc_sequences.values() {
                local_total_size += seq_store.compute_size();
                for bitmap in &seq_store.missing_symbol_bitmaps {
                    local_nucleotide_symbol_n_bitmaps_size += bitmap.get_size_in_bytes(false);
                }
            }
            sequence_count.fetch_add(database_partition.sequence_count, Ordering::SeqCst);
            total_size.fetch_add(local_total_size, Ordering::SeqCst);
            nucleotide_symbol_n_bitmaps_size.fetch_add(local_nucleotide_symbol_n_bitmaps_size, Ordering::SeqCst);
        });

        DatabaseInfo {
            sequence_count: sequence_count.load(Ordering::SeqCst),
            total_size: total_size.load(Ordering::SeqCst),
            n_bitmaps_size: nucleotide_symbol_n_bitmaps_size.load(Ordering::SeqCst),
            number_of_partitions: self.partitions.len(),
        }
    }
}
