#include "query_engine.h"

using namespace silo;

int handle_command(SequenceStore& db, MetaStore& meta_db, vector<string> args){
   const std::string default_db_filename = "../silo/roaring_sequences.silo";
   const std::string default_meta_filename = "../silo/meta_store.silo";
   if(args.empty()){
      return 0;
   }
   if("load" == args[0]){
      if(args.size() < 2) {
         std::cout << "Loading sequence_store from " << default_db_filename << std::endl;
         load_db(db, default_db_filename);
      }
      else if(args.size() == 2 && args[1].ends_with(".silo")){
         std::cout << "Loading sequence_store from " << args[1] << std::endl;
         load_db(db, args[1]);
      }
      else{
         cout << "Expected syntax: \"load [file_name.silo]\"" << endl;
      }
   }
   else if("save" == args[0]){
      if(args.size() < 2) {
         std::cout << "Saving sequence_store to " << default_db_filename << std::endl;
         save_db(db, default_db_filename);
      }
      else if(args.size() == 2 && args[1].ends_with(".silo")){
         std::cout << "Saving sequence_store to " << args[1] << std::endl;
         save_db(db, args[1]);
      }
      else{
         cout << "Expected syntax: \"save [file_name.silo]\"" << endl;
      }
   }
   else if("load_meta" == args[0]){
      if(args.size() < 2) {
         std::cout << "Loading meta_store from " << default_meta_filename << std::endl;
         load_meta(meta_db, default_db_filename);
      }
      else if(args.size() == 2 && args[1].ends_with(".silo")){
         std::cout << "Loading meta_store from " << args[1] << std::endl;
         load_meta(meta_db, args[1]);
      }
      else{
         cout << "Expected syntax: \"load_meta [file_name.silo]\"" << endl;
      }
   }
   else if("save_meta" == args[0]){
      if(args.size() < 2) {
         std::cout << "Saving meta_store to " << default_meta_filename << std::endl;
         save_meta(meta_db, default_meta_filename);
      }
      else if(args.size() == 2 && args[1].ends_with(".silo")){
         std::cout << "Saving meta_store to " << args[1] << std::endl;
         save_meta(meta_db, args[1]);
      }
      else{
         cout << "Expected syntax: \"save_meta [file_name.silo]\"" << endl;
      }
   }
   else if("info" == args[0]){
      db_info(db, cout);
   }
   else if("info_d" == args[0]){
      db_info_detailed(db, cout);
   }
   else if("meta_info" == args[0]){
      meta_info(meta_db, cout);
   }
   else if("benchmark" == args[0]){
      // benchmark(db);
   }
   else if ("build_raw" == args[0]){
      if(args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process_raw(db, cin);
      }
      else if(args.size() == 2){
         istream_wrapper file(args[1]);
         process_raw(db, file.get_is());
      }
      else{
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
      }
   }
   else if ("build" == args[0]){
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      if(args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process(db, meta_db, cin);
      }
      else if(args.size() == 2){
         istream_wrapper file(args[1]);
         process(db, meta_db, file.get_is());
      }
      else{
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
      }
   }
   else if ("calc_partition_offsets" == args[0]) {
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      if(args.size() < 2) {
         cout << "calc_partition_offsets from stdin." << endl;
         calc_partition_offsets(db, meta_db, cin);
      }
      else{
         istream_wrapper file(args[1]);
         calc_partition_offsets(db, meta_db, file.get_is());
      }
   }
   else if ("build_partitioned_otf" == args[0]) {
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      else if(db.pid_to_offset.empty()){
         cout << "Need to first calculate offsets. See 'calc_partition_offsets'."  << endl;
      }
      cout << "This clears all currently stored sequences. TODO no longer does this.\nPress (y) to continue." << endl;
      string s;
      cin >> s;
      if(s != "y" && s != "Y"){
         return 0;
      }
      if(args.size() < 2) {
         cout << "build_partitioned_otf from stdin." << endl;
         process_partitioned_on_the_fly(db, meta_db, cin);
      }
      else{
         istream_wrapper file(args[1]);
         process_partitioned_on_the_fly(db, meta_db, file.get_is());
      }
   }
   else if("partition" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"partition out_prefix [fasta_file | fasta_archive]\"" << endl;
         return 0;
      }
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      if(args.size() == 2){
         cout << "Partition sequence input from stdin" << endl;
         partition(meta_db, cin, args[1]);
      }
      else {
         istream_wrapper file(args[2]);
         partition(meta_db, file.get_is(), args[1]);
      }
      return 0;
   }
   else if ("build_partitioned" == args[0]) {
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_partitioned in_prefix\"" << endl;
         return 0;
      }
      const string in_prefix = args[1] + '_';
      for(auto& x : meta_db.pid_to_pango){
         ifstream in(in_prefix + x + ".fasta");
         process(db, meta_db, in);
      }
   }
   else if ("build_partitioned_c" == args[0]) {
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_partitioned_c in_prefix\"" << endl;
         return 0;
      }
      const string in_prefix = args[1] + '_';
      for(auto& x : meta_db.pid_to_pango){
         ifstream in(in_prefix + x + ".fasta.xz");
         boost::iostreams::filtering_istream archive;
         archive.push(boost::iostreams::lzma_decompressor());
         archive.push(in);
         cout << "Building sequence-store from input archive: " << args[1] << endl;
         process(db, meta_db, archive);
      }
   }
   else if ("analysemeta" == args[0]){
      if(args.size() == 1) {
         cout << "Analysing meta-data from stdin" << endl;
         analyseMeta(cin);
      }
      else {
         istream_wrapper file(args[1]);
         analyseMeta(file.get_is());
      }
   }
   else if ("build_meta_uns" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_meta meta_file\"" << endl;
         return 0;
      }

      istream_wrapper file(args[1]);
      processMeta(meta_db, file.get_is());
   }
   else if ("build_meta" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_meta METAFILE\"" << endl;
         return 0;
      }

      istream_wrapper file(args[1]);
      processMeta_ordered(meta_db, file.get_is());

   }
   else if ("exit" == args[0] || "quit" == args[0] ){
      return 1;
   }
   // ___________________________________________________________________________________________________________
   // From here on soon to be deprecated / altered. Compose these bigger commands differently using the above smaller ones
   else{
      cout << "Unknown command " << args[0] << "." << endl;
   }
   return 0;
}


int handle_command(SequenceStore& db, MetaStore& meta_db, const std::string& command_str){
   std::stringstream ss(command_str);
   std::istream_iterator<std::string> begin(ss);
   std::istream_iterator<std::string> end;
   std::vector<std::string> command(begin, end);
   return handle_command(db, meta_db, command);
}


int main(int argc, char* argv[]) {
   auto db = make_unique<SequenceStore>();
   auto meta_db = make_unique<MetaStore>();
   {
      if (argc >= 2) {
         for (int i = 1; i < argc; i++) {
            if (handle_command(*db, *meta_db, argv[i])) {
               return 0;
            }
         }
      }
      std::string s;
      cout << "> ";
      while (getline(std::cin, s)) {
         if (handle_command(*db, *meta_db, s)) {
            return 0;
         }
         cout << "> ";
      }
   }
   return 0;
}
