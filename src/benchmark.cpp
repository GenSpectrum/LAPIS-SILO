//
// Created by Alexander Taepper on 28.09.22.
//

#include "silo/query_engine.h"

using namespace silo;

void benchmark(const SequenceStore& db) {
   using system_clock = std::chrono::system_clock;

   std::string ref_genome;
   std::ifstream in("../Data/reference_genome.txt");

   if (!getline(in, ref_genome)) return;
   if (ref_genome.length() != genomeLength) {
      std::cerr << "length mismatch of reference!" << std::endl;
      return;
   }

   std::cout << "Q1: simple mutation filter and count" << std::endl;
   std::cout << "What is the number of sequences with the mutations G21987A and A22786C?" << std::endl;
   auto start = system_clock::now();
   // uint64_t q1res1 = db->bm(21987, Symbol::A)->cardinality();
   // uint64_t q1res1a = db->bma(21987, Residue::aA).cardinality();
   // uint64_t q1res2 = db->bm(22786,Symbol::C)->cardinality();
   // uint64_t q1res2a = db->bma(22786, Residue::aC).cardinality();
   uint64_t q1res = db.bm(21987, Symbol::A)->and_cardinality(*db.bm(22786, Symbol::C));
   uint64_t q1resa = db.bma(21987, Residue::aA).and_cardinality(db.bma(22786, Residue::aC));
   std::chrono::duration<double> elapsed_seconds = system_clock::now() - start;
   // std::cout << "Res1: " << q1res1 << " Res1 amb.: " << q1res1a << std::endl;
   // std::cout << "Res2: " << q1res2 << " Res2 amb.: " << q1res2a << std::endl;
   std::cout << "Res: " << q1res << " Res amb.: " << q1resa << std::endl;
   std::cout << "Computation took " << elapsed_seconds.count() << "seconds." << std::endl
             << std::endl;

   std::cout << "Q4 any mutation at given positions" << std::endl;
   std::cout << "What is the number of sequences where the following positions are mutated (i.e., not\n"
                "the same as the reference genome) or deleted? \n"
                "21618, 23948, 24424, 25000, 29510"
             << std::endl;

   start = system_clock::now();
   roaring::Roaring bms[5] = {db.bmr(21618, ref_genome), db.bmr(23948, ref_genome),
                              db.bmr(24424, ref_genome), db.bmr(25000, ref_genome),
                              db.bmr(29510, ref_genome)};
   for (auto& bm : bms) {
      bm.flip(0, db.sequenceCount);
   }
   const roaring::Roaring* inputs[5] = {&bms[0], &bms[1], &bms[2], &bms[3], &bms[4]};
   uint64_t res2 = db.sequenceCount - roaring::Roaring::fastunion(5, inputs).cardinality();
   elapsed_seconds = system_clock::now() - start;
   std::cout << res2 << std::endl;
   std::cout << "Computation took " << elapsed_seconds.count() << "seconds." << std::endl
             << std::endl;

   std::cout << "Q9 boolean algebra" << std::endl;

   start = system_clock::now();
   uint64_t res9 = (((db.ref_mut(21618, ref_genome)) & db.ref_mut(23984, ref_genome)) |
                    (db.ref_mut(21618, ref_genome) |
                     (db.neg_bm(23948, Symbol::T) | db.neg_bm(18163, Symbol::G))))
                      .cardinality();
   elapsed_seconds = system_clock::now() - start;
   std::cout << res9 << std::endl;
   std::cout << "Computation took " << elapsed_seconds.count() << "seconds." << std::endl
             << std::endl;
}
