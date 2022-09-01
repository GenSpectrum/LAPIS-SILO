#include "query_engine.cpp"


[[maybe_unused]] static unique_ptr<istream> maybe_xz_reader(const string& ending, const string& filename){
   if(filename.ends_with(ending)){
      return std::make_unique<ifstream>(filename);
   }
   else if(filename.ends_with(ending + ".xz")){
      std::ifstream file(filename, std::ios::binary);

      boost::iostreams::filtering_istreambuf in;
      in.push(boost::iostreams::lzma_decompressor());
      in.push(file);

      return std::make_unique<istream>(&in);
   }
   else{
      cerr << "Unrecognized file format (please specify "<< ending << " or " << ending << ".xz files as input)";
      cerr << "Received input filename: " << filename << endl;
      return nullptr;
   }
}

[[maybe_unused]] static unique_ptr<istream> fasta_reader(const string& fasta_filename){
   return maybe_xz_reader(".fasta", fasta_filename);
}

[[maybe_unused]] static unique_ptr<ostream> maybe_xz_writer(const string& ending, const string& filename){
   if(filename.ends_with(ending)){
      return std::make_unique<ofstream>(filename);
   }
   else if(filename.ends_with(ending + ".xz")){
      std::ofstream file(filename, std::ios::binary);

      boost::iostreams::filtering_ostreambuf in;
      in.push(boost::iostreams::lzma_compressor());
      in.push(file);

      return std::make_unique<ostream>(&in);
   }
   else{
      cerr << "Unrecognized file format (please specify " << ending << " or " << ending << ".xz files as output):";
      cerr << "Received input filename: " << filename << endl;
      return nullptr;
   }
}


[[maybe_unused]] static unique_ptr<ostream> fasta_writer(const string& fasta_filename){
   return maybe_xz_writer(".fasta", fasta_filename);
}




int handle_command(SequenceStore& db, MetaStore& meta_db, vector<string> args){
   const std::string default_db_filename = "../silo/roaring_sequences.silo";
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
         cout << "Expected syntax: \"load [file_name.silo]\"" << endl;
      }
   }
   else if("info" == args[0]){
      db_info(db, cout);
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
         unique_ptr<istream> in = fasta_reader(args[1]);
         if(in) {
            cout << "Building sequence-store from input file: " << args[1] << endl;
            process(db, *in);
         }
      }
      else{
         cout << "Expected syntax: \"build [fasta_file | fasta_archive]\"" << endl;
      }
   }
   else if("partition" == args[0]){
      // TODO
      return 0;
   }
   else if ("analysemeta" == args[0]){
      if(args.size() == 1) {
         cout << "Analysing meta-data from stdin" << endl;
         analyseMeta(cin);
      }
      else {
         cout << "Analysing meta-data from input file: " << args[1] << endl;
         unique_ptr<istream> in = fasta_reader(args[1]);
         if(in) {
            analyseMeta(*in);
         }
      }
   }
   else if ("build_meta" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_meta METAFILE\"" << endl;
      }

      cout << "Building meta-data indexes from input file: " << args[1] << endl;
      unique_ptr<istream> in = maybe_xz_reader("tsv", args[1]);

      if(!in){
         return 0;
      }

      unordered_map<uint64_t, uint16_t> epi_to_pid;
      vector<string> pid_to_pango;
      unordered_map<string, uint16_t> pango_to_pid;
      processMeta(*in, pid_to_pango, pango_to_pid, epi_to_pid);

      // Pango to partition is just mod 256 for now...

      vector<uint32_t> partition_to_offset;
      partition_to_offset.reserve(256);
      for(uint32_t i = 0; i<256; i++){
         partition_to_offset.push_back(i << 24);
      }
   }
   else if ("exit" == args[0] || "quit" == args[0] ){
      return 1;
   }
   // ___________________________________________________________________________________________________________
   // From here on soon to be deprecated / altered. Compose these bigger commands differently using the above smaller ones
   else if ("build_with_prefix" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"build_with_prefix METAFILE [SEQFILE]\"" << endl;
      }

      cout << "Building meta-data indexes from input file: " << args[1] << endl;
      unique_ptr<istream> in = fasta_reader(args[1]);

      unordered_map<uint64_t, uint16_t> epi_to_pid;
      vector<string> pid_to_pango;
      unordered_map<string, uint16_t> pango_to_pid;
      processMeta(*in, pid_to_pango, pango_to_pid, epi_to_pid);

      // Pango to partition is just mod 256 for now...

      vector<uint32_t> partition_to_offset;
      partition_to_offset.reserve(256);
      for(uint32_t i = 0; i<256; i++){
         partition_to_offset.push_back(i << 24);
      }

      cout << "Processed Meta" << endl;

      if(args.size() < 3){
         process_ordered(db, cin, epi_to_pid, partition_to_offset);
      }
      else {
         unique_ptr<istream> in2 = fasta_reader(args[2]);
         process_ordered(db, *in2, epi_to_pid, partition_to_offset);
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
