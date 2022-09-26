//
// Created by Alexander Taepper on 26.09.22.
//

#include "silo.h"


struct MetaStore{
   friend class boost::serialization::access;
   template<class Archive>
   void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
   {
      ar & pid_count;
      ar & epi_count;

      // vector
      ar & pid_to_pango;
      ar & pid_to_offset;

      // unordered_map
      ar & pango_to_pid;
      ar & epi_to_pid;
   }

   uint64_t epi_count = 0;
   uint16_t pid_count = 0;

   // Maps the epis to the ID, which is assigned to the pango lineage (pid)
   // pids are starting at 0 and are dense, so that we can save the respective data in vectors.
   unordered_map<uint64_t, uint16_t> epi_to_pid;

   vector<string> pid_to_pango;
   unordered_map<string, uint16_t> pango_to_pid;

   // pid to offsets
   vector<uint32_t> pid_to_offset;
};


// Meta-Data is input
void analyseMeta(istream& in);


void processMeta(MetaStore& mdb, istream& in);
void processMeta_ordered(MetaStore& mdb, istream& in);

void calc_partition_offsets(MetaStore& mdb, istream& in);

void meta_info(const MetaStore& mdb, ostream& out);

static unsigned save_meta(const MetaStore& db, const std::string& db_filename);

static unsigned load_meta(MetaStore& db, const std::string& db_filename);