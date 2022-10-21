#include "../test/query_test.cpp"
#include "silo/query_engine.h"

using namespace silo;

void info_messages() {
   using std::cin;
   using std::cout;
   using std::endl;
   cout << "SILO" << endl
        << endl;
   cout << "Usage:" << endl;
   cout << "\tsilo" << endl;
   cout << "\tStart silo in interactive mode" << endl
        << endl;
   cout << "\tsilo \"<command>\" ..." << endl;
   cout << "\tExecute the commands in the given order, then enter interactive mode." << endl
        << endl;
   cout << "\tCommands:" << endl
        << "\tbuild_meta [metadata.tsv]" << endl
        << "\tpartition <out_prefix> [fasta_archive]" << endl
        << "\tsort_partitions <io_prefix> [fasta_archive]" << endl
        << "\tbuild_meta [metadata.tsv]" << endl
        << "\tbuild [faste_archive]" << endl;
}

int handle_command(SequenceStore& db, MetaStore& mdb, std::vector<std::string> args) {
   using std::cin;
   using std::cout;
   using std::endl;

   const std::string default_db_filename = "../silo/roaring_sequences.silo";
   const std::string default_meta_filename = "../silo/meta_store.silo";
   if (args.empty()) {
      return 0;
   }
   if ("load" == args[0]) {
      if (args.size() < 2) {
         cout << "Loading sequence_store from " << default_db_filename << endl;
         load_db(db, default_db_filename);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Loading sequence_store from " << args[1] << endl;
         load_db(db, args[1]);
      } else {
         cout << "Expected syntax: \"load [file_name.silo]\"" << endl;
      }
   } else if ("save" == args[0]) {
      if (args.size() < 2) {
         cout << "Saving sequence_store to " << default_db_filename << endl;
         save_db(db, default_db_filename);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Saving sequence_store to " << args[1] << endl;
         save_db(db, args[1]);
      } else {
         cout << "Expected syntax: \"save [file_name.silo]\"" << endl;
      }
   } else if ("load_meta" == args[0]) {
      if (args.size() < 2) {
         cout << "Loading meta_store from " << default_meta_filename << endl;
         load_meta(mdb, default_meta_filename);
      } else if (args.size() == 2 && args[1].ends_with(".silo")) {
         cout << "Loading meta_store from " << args[1] << endl;
         load_meta(mdb, args[1]);
      } else {
         cout << "Expected syntax: \"load_meta [file_name.silo]\"" << endl;
      }
   } else if ("save_meta" == args[0]) {
      if (args.size() < 2) {
         cout << "Saving meta_store to " << default_meta_filename << endl;
         save_meta(mdb, default_meta_filename);
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
      // benchmark(db);
   } else if ("build_raw" == args[0]) {
      if (args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process_raw(db, cin);
      } else if (args.size() == 2) {
         istream_wrapper file(args[1]);
         process_raw(db, file.get_is());
      } else {
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
      }
   } else if ("build" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      if (args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process(db, mdb, cin);
      } else if (args.size() == 2) {
         istream_wrapper file(args[1]);
         process(db, mdb, file.get_is());
      } else {
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
      }
   } else if ("calc_partition_offsets" == args[0]) {
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      if (args.size() < 2) {
         cout << "calc_partition_offsets from stdin." << endl;
         calc_partition_offsets(db, mdb, cin);
      } else {
         istream_wrapper file(args[1]);
         calc_partition_offsets(db, mdb, file.get_is());
      }
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
      if (args.size() < 2) {
         cout << "build_partitioned_otf from stdin." << endl;
         process_partitioned_on_the_fly(db, mdb, cin);
      } else {
         istream_wrapper file(args[1]);
         process_partitioned_on_the_fly(db, mdb, file.get_is());
      }
   } else if ("partition" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"partition out_prefix [fasta_file | fasta_archive]\"" << endl;
         return 0;
      }
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      if (args.size() == 2) {
         cout << "Partition sequence input from stdin" << endl;
         partition_sequences(mdb, cin, args[1]);
      } else {
         istream_wrapper file(args[2]);
         partition_sequences(mdb, file.get_is(), args[1]);
      }
      return 0;
   } else if ("sort_partitions" == args[0]) {
      if (args.size() != 2) {
         cout << "Expected syntax: \"partition out_prefix\"" << endl;
         return 0;
      }
      if (mdb.epi_to_pid.empty()) {
         cout << "No meta_data built." << endl;
         return 0;
      }
      sort_partitions(mdb, args[1]);
      return 0;
   } else if ("build_partitioned" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"build_partitioned in_prefix\"" << endl;
         return 0;
      }
      const std::string in_prefix = args[1] + '_';
      for (unsigned i = 0; i < mdb.pangos.size(); i++) {
         std::ifstream in(in_prefix + std::to_string(i) + ".fasta");
         process(db, mdb, in);
      }
   } else if ("build_partitioned_c" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"build_partitioned_c in_prefix\"" << endl;
         return 0;
      }
      const std::string in_prefix = args[1] + '_';
      for (unsigned i = 0; i < mdb.pangos.size(); i++) {
         std::ifstream in(in_prefix + std::to_string(i) + ".fasta.xz");
         boost::iostreams::filtering_istream archive;
         archive.push(boost::iostreams::lzma_decompressor());
         archive.push(in);
         cout << "Building sequence-store from input archive: " << args[1] << endl;
         process(db, mdb, archive);
      }
   } else if ("build_meta_uns" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"build_meta meta_file\"" << endl;
         return 0;
      }

      istream_wrapper file(args[1]);
      processMeta(mdb, file.get_is());
   } else if ("build_meta" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"build_meta METAFILE\"" << endl;
         return 0;
      }

      istream_wrapper file(args[1]);
      processMeta_ordered(mdb, file.get_is());
      cout << "Loaded meta_data from file " << args[1] << endl;
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
   }
   // ___________________________________________________________________________________________________________
   // From here on soon to be deprecated / altered. Compose these bigger commands differently using the above smaller ones
   else {
      cout << "Unknown command " << args[0] << "." << endl;
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
      std::string s;
      std::cout << "> ";
      while (getline(std::cin, s)) {
         if (handle_command(*db, *mdb, s)) {
            return 0;
         }
         std::cout << "> ";
      }
   }
   return 0;
}
