#include "query_engine.h"

using namespace silo;

string test = "{\n"
"    \"type\": \"And\",\n"
"    \"children\": [\n"
"      {\n"
"        \"type\": \"StrEq\",\n"
"        \"column\": \"country\",\n"
"        \"value\": \"Switzerland\"\n"
"      },\n"
"      {\n"
"        \"type\": \"DateBetw\",\n"
"        \"from\": \"2022-01-01\",\n"
"        \"to\": null\n"
"      },\n"
"      {\n"
"        \"type\": \"Or\",\n"
"        \"children\": [\n"
"          {\n"
"            \"type\": \"NucEq\",\n"
"            \"position\": 25407,\n"
"            \"value\": \"A\"\n"
"          },\n"
"          {\n"
"            \"type\": \"Neg\",\n"
"            \"child\": {\n"
"              \"type\": \"PangoLineage\",\n"
"              \"column\": \"pango_lineage\",\n"
"              \"includeSubLineages\": true,\n"
"              \"value\": \"B.1.1.529\"\n"
"            }\n"
"          },\n"
"          {\n"
"            \"type\": \"N-Of\",\n"
"            \"n\": 2,\n"
"            \"exactly\": false,\n"
"            \"children\": [\n"
"              {\n"
"                \"type\": \"NucEq\",\n"
"                \"position\": 241,\n"
"                \"value\": \"T\"\n"
"              },\n"
"              {\n"
"                \"type\": \"NucEq\",\n"
"                \"position\": 29734,\n"
"                \"value\": \"T\"\n"
"              },\n"
"              {\n"
"                \"type\": \"NucEq\",\n"
"                \"position\": 28330,\n"
"                \"value\": \"G\"\n"
"              }\n"
"            ]\n"
"          }\n"
"        ]\n"
"      }\n"
"    ]\n"
"  }";



unique_ptr<Expression> silo::tag_invoke( boost::json::value_to_tag< unique_ptr<Expression>>, boost::json::value const& jv )
{
   // boost::algorithm::erase_all(test, " \n");
   boost::json::object const& js = jv.as_object();
   auto type = value_to<string>(js.at("type"));
   //type = type.substr(1, type.size() - 2);
   if(type == "And"){
      return make_unique<AndEx>(js);
   }
   else if(type == "Or"){
      return make_unique<OrEx>(js);
   }
   else if(type == "N-Of"){
      return make_unique<NOfEx>(js);
   }
   else if(type == "Neg"){
      return make_unique<NegEx>(js);
   }
   else if(type == "DateBetw"){
      return make_unique<DateBetwEx>(js);
   }
   else if(type == "NucEq"){
      return make_unique<NucEqEx>(js);
   }
   else if(type == "NucMut"){
      return make_unique<NucMutEx>(js);
   }
   else if(type == "PangoLineage"){
      return make_unique<PangoLineageEx>(js);
   }
   else if(type == "StrEq"){
      return make_unique<StrEqEx>(js);
   }
   else {
      throw std::runtime_error("Undefined type. Change this later to parsing exceptions.");
   }
}

Roaring* AndEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
   auto ret = start->evaluate(db, mdb);
   for(auto& child : children){
      auto bm = child->evaluate(db, mdb);
      *ret &= *bm;
      delete bm;
   }
   return ret;
}

Roaring* OrEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
   unsigned n = children.size();
   const Roaring* child_res[n];
   for(int i = 0; i<n; i++){
      child_res[i] = children[i]->evaluate(db, mdb);
   }
   auto ret = new Roaring(Roaring::fastunion(children.size(), child_res));
   for(int i = 0; i<n; i++){
      delete child_res[i];
   }
   return ret;
}

Roaring* NOfEx::evaluate(const silo::SequenceStore &db, const silo::MetaStore &mdb) {
   vector<uint16_t> count;
   count.resize(db.sequenceCount);
   for(auto & child : children){
      auto bm = child->evaluate(db, mdb);
      for(uint32_t id : *bm){
         count[id]++;
      }
      delete bm;
   }
   vector<uint32_t> correct;

   if(exactly){
      for(int i = 0; i< db.sequenceCount; i++){
         if(count[i] == n){
            correct.push_back(i);
         }
      }
   }
   else{
      for(int i = 0; i< db.sequenceCount; i++){
         if(count[i] >= n){
            correct.push_back(i);
         }
      }
   }

   return new Roaring(correct.size(), &correct[0]);
}

Roaring* NegEx::evaluate(const SequenceStore &db, const MetaStore &mdb){
   auto ret = child->evaluate(db, mdb);
   ret->flip(0, db.sequenceCount);
   return ret;
}

/*
Roaring* DateBetwEx::evaluate(const SequenceStore &db, const MetaStore &mdb){
   if(open_from && open_to){
      auto ret = new Roaring;
      ret->flip(0, db.sequenceCount);
      return ret;
   }
   vector<uint32_t> seqs;

   if(open_from){

   }
   else if(open_to){

   }
   else{

   }

   return new Roaring(seqs.size(), &seqs[0]);
}*/


Roaring* NucEqEx::evaluate(const SequenceStore &db, const MetaStore &mdb){
   return new Roaring(*db.bm(position, to_symbol(value)));
}

int testmain(){
   /*boost::json::parse_options opt;
   opt.allow_comments = true;
   opt.allow_trailing_commas = true;*/

   const boost::json::value& jv = boost::json::parse(test);
   const boost::json::object& obj = jv.as_object();
   auto x =  value_to<unique_ptr<Expression>>(jv);
   cout << x;

   return 0;
}

void benchmark(const SequenceStore& db)
{

   string ref_genome;
   ifstream in("../Data/reference_genome.txt");

   if (!getline(in, ref_genome)) return;
   if (ref_genome.length() != genomeLength) {
      cerr << "length mismatch of reference!" << endl;
      return;
   }


   cout << "Q1: simple mutation filter and count" << endl;
   cout << "What is the number of sequences with the mutations G21987A and A22786C?" << endl;
   auto start = chrono::system_clock::now();
   // uint64_t q1res1 = db->bm(21987, Symbol::A)->cardinality();
   // uint64_t q1res1a = db->bma(21987, Residue::aA).cardinality();
   // uint64_t q1res2 = db->bm(22786,Symbol::C)->cardinality();
   // uint64_t q1res2a = db->bma(22786, Residue::aC).cardinality();
   uint64_t q1res = db.bm(21987, Symbol::A)->and_cardinality(*db.bm(22786,Symbol::C));
   uint64_t q1resa = db.bma(21987, Residue::aA).and_cardinality(db.bma(22786, Residue::aC));
   chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-start;
   // cout << "Res1: " << q1res1 << " Res1 amb.: " << q1res1a << endl;
   // cout << "Res2: " << q1res2 << " Res2 amb.: " << q1res2a << endl;
   cout << "Res: " << q1res << " Res amb.: " << q1resa << endl;
   cout << "Computation took " << elapsed_seconds.count() << "seconds." << endl << endl;

   cout << "Q4 any mutation at given positions" << endl;
   cout << "What is the number of sequences where the following positions are mutated (i.e., not\n"
           "the same as the reference genome) or deleted? \n"
           "21618, 23948, 24424, 25000, 29510" << endl;

   start = chrono::system_clock::now();
   roaring::Roaring bms[5] = {db.bmr(21618, ref_genome), db.bmr(23948, ref_genome),
                              db.bmr(24424, ref_genome), db.bmr(25000, ref_genome),
                              db.bmr(29510, ref_genome)};
   for(auto & bm : bms){
      bm.flip(0, db.sequenceCount);
   }
   const roaring::Roaring *inputs[5] = {&bms[0],&bms[1],&bms[2],&bms[3],&bms[4]};
   uint64_t res2 = db.sequenceCount - roaring::Roaring::fastunion(5, inputs).cardinality();
   elapsed_seconds = chrono::system_clock::now()-start;
   cout << res2 << endl;
   cout << "Computation took " << elapsed_seconds.count() << "seconds." << endl << endl;


   cout << "Q9 boolean algebra" << endl;

   start = chrono::system_clock::now();
   uint64_t res9 = (((db.ref_mut(21618, ref_genome)) & db.ref_mut(23984, ref_genome)) |
                    (db.ref_mut(21618, ref_genome) | (db.neg_bm(23948, Symbol::T) | db.neg_bm(18163, Symbol::G)))).cardinality();
   elapsed_seconds = chrono::system_clock::now()-start;
   cout << res9 << endl;
   cout << "Computation took " << elapsed_seconds.count() << "seconds." << endl << endl;

}
