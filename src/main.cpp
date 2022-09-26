#include "query_engine.cpp"



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
   if("load_meta" == args[0]){
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
         save_meta(meta_db, default_db_filename);
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
   else if("meta_info" == args[0]){
      meta_info(meta_db, cout);
   }
   else if("benchmark" == args[0]){
      benchmark(db);
   }
   else if ("build" == args[0]){
      if(args.size() == 1) {
         cout << "Building sequence-store from stdin" << endl;
         process(db, cin);
      }
      else if(args.size() == 2){
         auto file = ifstream(args[1], ios::binary);
         if(args[1].ends_with(".xz")){
            xzistream archive;
            archive.push(boost::iostreams::lzma_decompressor());
            archive.push(file);
            cout << "Building sequence-store from input archive: " << args[1] << endl;
            process(db, archive);
         }
         else {
            cout << "Building sequence-store from input file: " << args[1] << endl;
            process(db, file);
         }
      }
      else{
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
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
         auto file = ifstream(args[2], ios::binary);
         if(args[2].ends_with(".xz")){
            xzistream archive;
            archive.push(boost::iostreams::lzma_decompressor());
            archive.push(file);
            cout << "Partition sequence input from input archive: " << args[2] << endl;
            partition(meta_db, archive, args[1]);
         }
         else {
            cout << "Partition sequence input from input file: " << args[2] << endl;
            partition(meta_db, file, args[1]);
         }
      }
      return 0;
   }
   else if ("calc_partition_offsets" == args[0]) {
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      if(args.size() < 2) {
         calc_partition_offsets(meta_db, cin);
      }
      else{
         auto file = ifstream(args[2], ios::binary);
         if(args[2].ends_with(".xz")){
            xzistream archive;
            archive.push(boost::iostreams::lzma_decompressor());
            archive.push(file);
            cout << "Partition sequence input from input archive: " << args[2] << endl;
            calc_partition_offsets(meta_db, archive);
         }
         else {
            cout << "Partition sequence input from input file: " << args[2] << endl;
            calc_partition_offsets(meta_db, file);
         }
      }
   }
   else if ("build_partitioned_otf" == args[0]) {
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      // TODO
   }
   else if ("build_partitioned" == args[0]) {
      if(meta_db.epi_to_pid.empty()){
         cout << "No meta_data built."  << endl;
         return 0;
      }
      // TODO
   }
   else if ("analysemeta" == args[0]){
      if(args.size() == 1) {
         cout << "Analysing meta-data from stdin" << endl;
         analyseMeta(cin);
      }
      else {
         auto file = ifstream(args[1], ios::binary);
         if(args[1].ends_with(".xz")){
            xzistream archive;
            archive.push(boost::iostreams::lzma_decompressor());
            archive.push(file);
            cout << "Analysing meta-data from input archive: " << args[1] << endl;
            analyseMeta(archive);
         }
         else {
            cout << "Analysing meta-data from input file: " << args[1] << endl;
            analyseMeta(file);
         }
      }
   }
   else if ("build_meta" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_meta METAFILE\"" << endl;
         return 0;
      }


      auto file = ifstream(args[1], ios::binary);
      if(args[1].ends_with(".xz")){
         xzistream archive;
         archive.push(boost::iostreams::lzma_decompressor());
         archive.push(file);
         cout << "Building meta-data indexes from input archive: " << args[1] << endl;
         processMeta(meta_db, archive);
      }
      else {
         cout << "Building meta-data indexes from input file: " << args[1] << endl;
         processMeta(meta_db, file);
      }
   }
   else if ("exit" == args[0] || "quit" == args[0] ){
      return 1;
   }
   // ___________________________________________________________________________________________________________
   // From here on soon to be deprecated / altered. Compose these bigger commands differently using the above smaller ones
   else if("test" == args[0]){
      std::ifstream file("../tmp.fasta.xz", std::ios::binary);



      boost::iostreams::filtering_istream in2;
      in2.push(boost::iostreams::lzma_decompressor());
      in2.push(file);

      istream& in = in2;

      if(!in) {
         cout << "Could not create in_stream."  << endl;
         return 0;
      }
      cout << "Reading." << endl;
      string s;
      istream& tmp = in;
      getline(tmp,s);
      cout << "Read:'" <<  s << "'" << endl;
      return 0;
   }
   else if ("build_with_prefix" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_with_prefix METAFILE [SEQFILE]\"" << endl;
      }

      auto file = ifstream(args[1], ios::binary);
      if(args[1].ends_with(".xz")){
         xzistream archive;
         archive.push(boost::iostreams::lzma_decompressor());
         archive.push(file);
         cout << "Building meta-data indexes from input archive: " << args[1] << endl;
         processMeta(meta_db, archive);
      }
      else {
         cout << "Building meta-data indexes from input file: " << args[1] << endl;
         processMeta(meta_db, file);
      }

      // Pango to partition is just mod 256 for now...
      meta_db.partition_to_offset.clear();
      meta_db.partition_to_offset.reserve(256);

      for(uint32_t i = 0; i<256; i++){
         meta_db.partition_to_offset.push_back(i << 24);
      }

      cout << "Processed Meta" << endl;

      if(args.size() < 3){
         process_ordered(db, cin, meta_db.epi_to_pid, meta_db.partition_to_offset);
      }
      else {

         auto file2 = ifstream(args[2], ios::binary);
         if(args[2].ends_with(".xz")){
            xzistream archive;
            archive.push(boost::iostreams::lzma_decompressor());
            archive.push(file);
            cout << "Processing sequences with on-the-fly partitioning from input archive: " << args[2] << endl;
            process_ordered(db, archive, meta_db.epi_to_pid, meta_db.partition_to_offset);
         }
         else {
            cout << "Processing sequences with on-the-fly partitioning from input file: " << args[2] << endl;
            process_ordered(db, file2, meta_db.epi_to_pid, meta_db.partition_to_offset);
         }
      }

      db_info(db, cout);
      benchmark(db);
   }
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
   cout << "All objects destroyed. Exiting." << endl;
   return 0;
}
