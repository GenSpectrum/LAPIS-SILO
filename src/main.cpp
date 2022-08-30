#include "benchmark.cpp"


static void interpret(Database& db, const vector<string>& genomes);

static unique_ptr<Database> process(istream& in) {
   static constexpr unsigned chunkSize = 1024;

   unique_ptr<Database> db = make_unique<Database>();
   vector<string> genomes;
   while (true) {
      string name, genome;
      if (!getline(in, name)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return nullptr;
      }
      genomes.push_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret(*db, genomes);
         genomes.clear();
      }
   }
   interpret(*db, genomes);
   cout << "sequence count: " << db->sequenceCount << endl;
   cout << "total size: " << db->computeSize() << endl;
   return db;
}

static void interpret(Database& db, const vector<string>& genomes){
   vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
         char c = genomes[index2][index];
         Symbol s = to_symbol(c);
         offsets[s].push_back(db.sequenceCount + index2);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2)
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
   }
   db.sequenceCount += genomes.size();
}


static void interpret_ordered(Database& db, const vector<pair<uint64_t, string>>& genomes);

static unique_ptr<Database> process_ordered(istream& in, unordered_map<uint64_t, uint16_t> epi_to_pango) {
   static constexpr unsigned chunkSize = 1024;

   unique_ptr<Database> db = make_unique<Database>();
   vector<pair<uint64_t, string>> genomes;
   while (true) {
      string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         cerr << "length mismatch!" << endl;
         return nullptr;
      }
      uint64_t epi = stoi(epi_isl.substr(7));
      genomes.emplace_back(((uint64_t) epi_to_pango[epi] << 48), std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret_ordered(*db, genomes);
         genomes.clear();
      }
   }
   interpret_ordered(*db, genomes);
   cout << "sequence count: " << db->sequenceCount << endl;
   cout << "total size: " << db->computeSize() << endl;
   return db;
}

static void interpret_ordered(Database& db, const vector<pair<uint64_t, string>>& genomes){
   vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
         char c = genomes[index2].second[index];
         Symbol s = to_symbol(c);
         // genomes[index2].first is the prefix (highest 16 bits), so we guarantee ordering by pango lineage
         offsets[s].push_back(genomes[index2].first + db.sequenceCount + index2);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2) {
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
      }
   }
   db.sequenceCount += genomes.size();
}

static string getPangoPrefix(const string &pango_lineage){
   string pangoPref;
   if(pango_lineage.size() > 2){
      std::stringstream ss(pango_lineage);
      if(!getline(ss, pangoPref, '.')){
         cerr << "Non-covered case of pango lineage!" << endl;
         return "Not-recognized";
      }
   }
   else{
      pangoPref = pango_lineage;
   }
   return pangoPref;
}


// Meta-Data is input
void processMeta(istream& in){
   in.ignore(LONG_MAX, '\n');

   // vector<string> lineage_vec;
   std::unordered_map<string, uint32_t> lineages;
   while (true) {
      string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      /*if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;*/
      if (!getline(in, division, '\n')) break;

      const string& pangoPref = getPangoPrefix(pango_lineage);

      if (!lineages.contains(pangoPref)) {
         lineages[pangoPref] = 1;
      }
      else{
         lineages[pangoPref]++;
      }
   }

   cout << "total lineages: " << lineages.size() << endl;

   for (auto &x: lineages)
      std::cout << x.first << ':' << x.second << '\n';
   cout << "total lineages: " << lineages.size() << endl;
}


// Meta-Data is input
void processMeta(istream& in, vector<string> i_to_pango, unordered_map<string, uint16_t> pango_to_i,
                 unordered_map<uint64_t, uint16_t> epi_to_pango){
   in.ignore(LONG_MAX, '\n');

   uint16_t idx = 0;
   while (true) {
      string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      /*if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;*/
      if (!getline(in, division, '\n')) break;

      uint64_t epi = stoi(epi_isl.substr(7));
      uint16_t pango_idx;
      if(pango_to_i.contains(pango_lineage)){
         pango_idx = pango_to_i[pango_lineage];
      }
      else{
         pango_idx = idx++;
         i_to_pango.push_back(pango_lineage);
         pango_to_i[pango_lineage] = pango_idx;
      }
      epi_to_pango[epi] = pango_idx;
   }

   cout << "Built Meta-indices." << endl;
}

int handle_command(vector<string> args){
   const std::string db_filename = "../out/roaring_sequences.silo";
   if(args.empty()){
      return -1;
   }
   if("load" == args[0]){
      unique_ptr<Database> db = make_unique<Database>();
      std::cout << "Loading index from " << db_filename << std::endl;
      load_db(db.get(), db_filename);
      db_info(db, cout);
      benchmark(db);
   }
   else if ("build" == args[0]){
      unique_ptr<Database> db;
      if(args.size() == 1) {
         cout << "Building indexes from stdin" << endl;
         db = process(cin);
         if (db) {
            save_db(*db, db_filename);
         }
      }
      else {
         cout << "Building indexes from input file: " << args[1] << endl;
         ifstream in(args[1]);
         db = process(in);
      }
      if (db) {
         save_db(*db, db_filename);
      }
      /*ifstream file(argv[1]);
      boost::iostreams::filtering_istream in;
      in.push(boost::iostreams::lzma_decompressor());
      in.push(file); */
   }
   else if ("buildmeta" == args[0]){
      if(args.size() == 1) {
         cout << "Building meta-data indexes from stdin" << endl;
         processMeta(cin);
      }
      else {
         cout << "Building meta-data indexes from input file: " << args[1] << endl;
         ifstream in(args[1]);
         processMeta(in);
      }
   }
   else if ("test" == args[0]){
      if(args.size() < 2) {
         cout << "Expected syntax: \"test METAFILE\"" << endl;
      }
      else {
         cout << "Building meta-data indexes from input file: " << args[1] << endl;
         ifstream in(args[1]);
         unordered_map<uint64_t, uint16_t> epi_to_pango;
         vector<string> i_to_pango;
         unordered_map<string, uint16_t> pango_to_i;
         processMeta(in, i_to_pango, pango_to_i, epi_to_pango);
      }
   }
   else{
      cout << "Unknown command " << args[0] << ".\n." << endl;
      return -1;
   }
   return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
       std::string s;
       while(getline(std::cin, s)) {
          std::stringstream ss(s);
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> vstrings(begin, end);
          handle_command(vstrings);
       }
    } else {
       vector<string> args;
       for(unsigned i = 1; i<argc; i++){
          args.emplace_back(argv[i]);
       }
       handle_command(args);
   }
   return 0;
}
