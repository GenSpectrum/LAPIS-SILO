//
// Created by Alexander Taepper on 02.02.23.
//

#include <silo/bootstrap.h>
#include <silo/query_engine/query_engine.h>

unsigned gen_start_N() {
   unsigned r = rand() % 14495618;
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count, second is bucket upper
      {
         {2256125, 25},
         {5840168, 45},
         {12678885, 55},
         {12796712, 70},
         {12948586, 80},
         {13343882, 150},
         {14067025, 400},
         {14243091, 1000},
         {14271355, 10000},
         {14271938, 21000},
         {14480991, 22000},
         {14495618, 29903},
      };
   if (r < 328928) { /// # == 0
      return 0;
   }
   std::pair<unsigned, unsigned> last_br = {328928, 0};
   for (auto br : breaks) {
      if (r < br.first) { /// # < br
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldnt happen, passed last break 1 " << std::to_string(r) << std::endl;
   return 1;
}

unsigned gen_end_N() {
   unsigned r = rand() % 14495618;
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count, second is bucket upper
      {
         {1823567, 25},
         {2898370, 45},
         {3921896, 55},
         {8623606, 70},
         {11335333, 80},
         {12994598, 150},
         {14180836, 400},
         {14256142, 1000},
         {14492195, 10000},
         {14493096, 21000},
         {14495440, 22000},
         {14495618, 29903},
      };
   if (r < 1823567) { /// # == 0
      return 0;
   }
   std::pair<unsigned, unsigned> last_br = {1525153, 0};
   for (auto br : breaks) {
      if (r < br.first) { /// # < br
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldnt happen, passed last break 2 " << std::to_string(r) << std::endl;
   return 1;
}

unsigned gen_N_len() {
   unsigned r = rand() % 77869427;
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count, second is bucket upper
      {
         {43610073, 25},
         {50662078, 45},
         {52933729, 55},
         {55229971, 70},
         {56557439, 80},
         {62160715, 150},
         {75918742, 400},
         {77644352, 1000},
         {77869380, 10000},
         {77869414, 21000},
         {77869415, 22000},
         {77869427, 29903},
      };
   if (r < 15439497) { /// # == 1
      return 1;
   }
   std::pair<unsigned, unsigned> last_br = {15439497, 1};
   for (auto br : breaks) {
      if (r < br.first) { /// # < 25
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldnt happen, passed last break 3 " << std::to_string(r) << std::endl;
   return 1;
}

char sample_pos(std::vector<uint32_t>& dist) {
   unsigned r = rand() % dist.back();
   for (unsigned symbol = 0; symbol < dist.size(); ++symbol) {
      if (r < dist[symbol]) {
         return silo::symbol_rep[symbol];
      }
   }
}

void gen_genome(char* ret, std::vector<std::vector<uint32_t>>& dist) {
   unsigned start_N = gen_start_N();
   unsigned end_N = gen_end_N();
   unsigned num_inner_N_runs = rand() % 12;

   for (unsigned i = 0; i < start_N; ++i)
      ret[i] = 'N';
   for (unsigned i = silo::genomeLength - end_N; i < silo::genomeLength; ++i)
      ret[i] = 'N';

   for (unsigned pos = start_N; pos < silo::genomeLength - end_N; ++pos) {
      ret[pos] = sample_pos(dist[pos]);
   }

   if (start_N + end_N + 2 >= silo::genomeLength)
      return;

   unsigned inner_len = silo::genomeLength - (start_N + end_N + 2);
   for (unsigned N_run = 0; N_run < num_inner_N_runs; ++N_run) {
      unsigned N_run_length = gen_N_len();
      if (inner_len <= N_run_length) {
         continue;
      }
      unsigned offset = (rand() % (inner_len - N_run_length)) + start_N + 1;
      for (unsigned i = offset; i < offset + N_run_length; ++i) {
         ret[i] = 'N';
      }
   }
}

int bootstrap(const silo::Database& db, std::string& out_dir, unsigned seed, unsigned factor) {
   srand(seed);
   unsigned pango_count = db.dict->get_pango_count();
   for (unsigned pango_id = 0; pango_id < pango_count; pango_id++) {
      auto predicate = std::make_unique<silo::PangoLineageEx>(pango_id, false);
      auto part_filters = silo::execute_predicate(db, predicate.get());
      unsigned count = 0;
      for (auto& f : part_filters) count += f.getAsConst()->cardinality();
      auto dist = silo::execute_all_dist(db, part_filters);
      for (unsigned rep = 0; rep < factor; rep++) {
         std::string file_name = out_dir + std::to_string(pango_id) + "_" + std::to_string(rep) + "_" + db.dict->get_pango(pango_id) + ".fasta";
         /// Generate output file
         {
            std::ofstream out_file(file_name);
            if (!out_file || out_file.fail()) {
               std::cerr << "Could not open output file " << file_name << std::endl;
               return -1;
            }
            for (unsigned i = 0; i < count; i++) {
               out_file << "EPI_ISL_" << std::to_string(pango_id) << "_" << std::to_string(rep) << "_" << std::to_string(i) << '\n';
               char genome[silo::genomeLength];
               gen_genome(genome, dist);
               out_file << genome << '\n';
            }
         }
         /// xz compress output file
         {
            std::string compress_command = "xz -t 0 " + file_name;
            system(compress_command.c_str());
         }
      }
   }
   return 0;
}
