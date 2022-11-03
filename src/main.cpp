#include "../test/query_test.cpp"
#include "silo/query_engine.h"

#include <readline/history.h>
#include <readline/readline.h>

using namespace silo;

void info_message() {
   using std::cin;
   using std::cout;
   using std::endl;
   cout << "SILO - Sequence Indexing engine for aLigned Ordered genomic data" << endl
        << endl;
   cout << "Usage:" << endl;
   cout << "\tsilo" << endl;
   cout << "\tStart silo in interactive mode" << endl
        << endl;
   cout << "\tsilo \"<command>\" ..." << endl;
   cout << "\tExecute the commands in the given order, then enter interactive mode." << endl
        << endl;
   cout << "\tCommands:" << endl
        << "\tbuild [fasta_archive]" << endl
        << "\tbuild_meta [metadata.tsv]" << endl
        << "\tpartition <out_prefix> [fasta_archive]" << endl
        << "\tsort_partitions <io_prefix> [fasta_archive]" << endl
        << "\tbuild_partitioned [part_prefix] [part_suffix]" << endl;
}

int handle_command(SequenceStore& db, MetaStore& mdb, std::vector<std::string> args) {
   using std::cin;
   using std::cout;
   using std::endl;

   const std::string default_db_savefile = "../silo/roaring_sequences.silo";
   const std::string default_meta_savefile = "../silo/meta_store.silo";
   const std::string default_sequence_input = "../Data/aligned.fasta.xz";
   const std::string default_metadata_input = "../Data/metadata.tsv";
   const std::string default_partition_prefix = "../Data/Partitioned/aligned";
   if (args.empty()) {
      return 0;
   }
   if ("load" == args[0]) {
      if (args.size() < 2) {
         cout << "Loading sequence_store from " << default_db_savefile << endl;
         load_db(db, default_db_savefile);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Loading sequence_store from " << args[1] << endl;
         load_db(db, args[1]);
      } else {
         cout << "Expected syntax: \"load [file_name.silo]\"" << endl;
      }
   } else if ("save" == args[0]) {
      if (args.size() < 2) {
         cout << "Saving sequence_store to " << default_db_savefile << endl;
         save_db(db, default_db_savefile);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Saving sequence_store to " << args[1] << endl;
         save_db(db, args[1]);
      } else {
         cout << "Expected syntax: \"save [file_name.silo]\"" << endl;
      }
   } else if ("load_meta" == args[0]) {
      if (args.size() < 2) {
         cout << "Loading meta_store from " << default_meta_savefile << endl;
         load_meta(mdb, default_meta_savefile);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Loading meta_store from " << args[1] << endl;
         load_meta(mdb, args[1]);
      } else {
         cout << "Expected syntax: \"load_meta [file_name.silo]\"" << endl;
      }
   } else if ("save_meta" == args[0]) {
      if (args.size() < 2) {
         cout << "Saving meta_store to " << default_meta_savefile << endl;
         save_meta(mdb, default_meta_savefile);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Saving meta_store to " << args[1] << endl;
         save_meta(mdb, args[1]);
      } else {
         cout << "Expected syntax: \"save_meta [file_name.silo]\"" << endl;
      }
   } else if ("info" == args[0]) {
      db_info(db, cout);
   } else if ("info_d" == args[0]) {
      db_info_detailed(db, cout);
   } else if ("pango_info" == args[0]) {
      pango_info(mdb, cout);
   } else if ("partition_info" == args[0]) {
      partition_info(mdb, cout);
   } else if ("benchmark" == args[0]) {
      cout << "Unavailable." << endl;
      // benchmark(db);
   } else if ("runoptimize" == args[0]) {
      cout << runoptimize(db) << endl;
   } else if ("build_raw" == args[0]) {
      if (args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process_raw(db, cin);
      } else if (args.size() == 2) {
         istream_wrapper file(args[1]);
         process_raw(db, file.get_is());
      } else {
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
         info_message();
         return 0;
      }
   } else if ("build" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      std::string inputfile;
      if (args.size() == 1) {
         inputfile = default_sequence_input;
      } else if (args.size() == 2) {
         inputfile = args[1];
      } else {
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
         return 0;
      }
      cout << "Building sequence-store from " << inputfile << endl;
      istream_wrapper file(inputfile);
      process(db, mdb, file.get_is());
   } else if ("calc_partition_offsets" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      std::string inputfile;
      if (args.size() == 1) {
         inputfile = default_sequence_input;
      } else if (args.size() == 2) {
         inputfile = args[1];
      } else {
         cout << "Expected syntax: \"calc_partition_offsets [fasta_file | fasta_archive]\"" << endl;
         return 0;
      }
      cout << "calc_partition_offsets from " << inputfile << endl;
      istream_wrapper file(inputfile);
      calc_partition_offsets(db, mdb, file.get_is());
   } else if ("build_partitioned_otf" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      } else if (db.part_to_offset.empty()) {
         cout << "Need to first calculate offsets. See 'calc_partition_offsets'." << endl;
      }
      cout << "This clears all currently stored sequences. TODO no longer does this.\nPress (y) to continue." << endl;
      std::string s;
      cin >> s;
      if (s != "y" && s != "Y") {
         return 0;
      }
      std::string inputfile;
      if (args.size() == 1) {
         inputfile = default_sequence_input;
      } else if (args.size() == 2) {
         inputfile = args[1];
      } else {
         cout << "Expected syntax: \"calc_partition_offsets [fasta_file | fasta_archive]\"" << endl;
         return 0;
      }
      cout << "build_partitioned_otf from " << inputfile << endl;
      istream_wrapper file(inputfile);
      process_partitioned_on_the_fly(db, mdb, file.get_is());
   } else if ("partition" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      std::string inputfile, part_prefix;
      if (args.size() == 1) {
         inputfile = default_sequence_input;
         part_prefix = default_partition_prefix;
      } else if (args.size() == 2) {
         inputfile = args[1];
         part_prefix = default_partition_prefix;
      } else if (args.size() == 3) {
         inputfile = args[1];
         part_prefix = args[2];
      } else {
         cout << "Expected syntax: \"partition [fasta_file | fasta_archive] [out_prefix]\"" << endl;
         return 0;
      }
      cout << "build_partitioned_otf from " << inputfile << " into " << part_prefix << endl;
      istream_wrapper file(inputfile);
      partition_sequences(mdb, file.get_is(), part_prefix);
      return 0;
   } else if ("sort_partitions" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      std::string part_prefix;
      if (args.size() == 1) {
         part_prefix = default_partition_prefix;
      } else if (args.size() == 2) {
         part_prefix = args[1];
      } else {
         cout << "Expected syntax: \"partition [out_prefix]\"" << endl;
         return 0;
      }
      cout << "sort_partitions in " << part_prefix << endl;
      sort_partitions(mdb, part_prefix);
      return 0;
   } else if ("build_partitioned" == args[0]) {
      std::string part_prefix, part_suffix;
      if (args.size() == 1) {
         part_prefix = default_partition_prefix;
         part_suffix = ".fasta";
      } else if (args.size() == 2) {
         part_prefix = args[1];
         part_suffix = ".fasta";
      } else if (args.size() == 3) {
         part_prefix = args[1];
         part_suffix = args[2];
      } else {
         cout << "Expected syntax: \"build_partitioned [partition_prefix] [partition_suffix]\"" << endl;
         return 0;
      }
      for (unsigned i = 0; i < mdb.partitions.size(); i++) {
         std::string name = part_prefix + std::to_string(i);
         name += part_suffix;
         istream_wrapper in(name);
         cout << "Building sequence-store from input file: " << name << endl;
         process(db, mdb, in.get_is());
      }
   } else if ("build_meta_uns" == args[0]) {
      std::string meta_file;
      if (args.size() == 1) {
         meta_file = default_metadata_input;
      } else if (args.size() == 2) {
         meta_file = args[1];
      } else {
         cout << "Expected syntax: \"build_meta_uns [meta_file]\"" << endl;
         return 0;
      }

      cout << "Building meta_data (unsorted) from file " << meta_file << endl;
      istream_wrapper file(meta_file);
      processMeta(mdb, file.get_is());
   } else if ("build_meta" == args[0]) {
      std::string meta_file;
      if (args.size() == 1) {
         meta_file = default_metadata_input;
      } else if (args.size() == 2) {
         meta_file = args[1];
      } else {
         cout << "Expected syntax: \"build_meta [meta_file]\"" << endl;
         return 0;
      }

      cout << "Building meta_data from file " << meta_file << endl;
      istream_wrapper file(meta_file);
      processMeta_ordered(mdb, file.get_is());
      cout << "Finished building meta_data from file " << meta_file << endl;
   } else if ("query" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"query JSON_QUERY\"" << endl;
         return 0;
      }

      std::stringstream query2;
      query2 << "{\n"
                "  \"action\": {\n"
                "    \"type\": \"Aggregated\",\n"
                "    \"groupByFields\": [\n"
                "      \"date\",\n"
                "      \"division\"\n"
                "    ]\n"
                "  },  \n"
                "\n"
                "  \"filter\": "
             << filter3 << "}";
      cout << execute_query(db, mdb, query2.str()) << endl;
   } else if ("exit" == args[0] || "quit" == args[0]) {
      return 1;
   } else if ("help" == args[0] || "-h" == args[0] || "--help" == args[0]) {
      info_message();
   }
   // ___________________________________________________________________________________________________________
   // From here on soon to be deprecated / altered. Compose these bigger commands differently using the above smaller ones
   else {
      cout << "Unknown command " << args[0] << "." << endl;
      cout << "Type 'help' for additional information." << endl;
      return 0;
   }
   return 0;
}

int handle_command(SequenceStore& db, MetaStore& mdb, const std::string& command_str) {
   std::stringstream ss(command_str);
   std::istream_iterator<std::string> begin(ss);
   std::istream_iterator<std::string> end;
   std::vector<std::string> command(begin, end);
   return handle_command(db, mdb, command);
}

int main(int argc, char* argv[]) {
   auto db = std::make_unique<SequenceStore>();
   auto mdb = std::make_unique<MetaStore>();
   {
      if (argc >= 2) {
         for (int i = 1; i < argc; i++) {
            if (handle_command(*db, *mdb, argv[i])) {
               return 0;
            }
         }
      }

      char* buf;
      while ((buf = readline(">> ")) != nullptr) {
         if (strlen(buf) > 0) {
            add_history(buf);

            if (handle_command(*db, *mdb, std::string(buf))) {
               return 0;
            }
         }
         // readline malloc's a new buffer every time.
         free(buf);
      }
   }
   return 0;
}
