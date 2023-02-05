//
// Created by Alexander Taepper on 02.02.23.
//

#include <silo/bootstrap.h>
#include <silo/common/lehmer64.h>
#include <silo/query_engine/query_engine.h>
#include <tbb/parallel_for.h>

uint64_t my_random(uint64_t s) {
   uint64_t x = lehmer64();
   __uint128_t m = (__uint128_t) x * (__uint128_t) s;
   uint64_t l = (uint64_t) m;
   if (l < s) {
      uint64_t t = -s % s;
      while (l < t) {
         x = lehmer64();
         m = (__uint128_t) x * (__uint128_t) s;
         l = (uint64_t) m;
      }
   }
   return m >> 64;
}

unsigned gen_start_N() {
   unsigned r = my_random(14495618);
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count in bucket, second is bucket upper limit
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
      if (r < br.first) { /// # < br -> interpolate
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldn't happen, passed last break 1 " << std::to_string(r) << std::endl;
   return 1;
}

unsigned gen_end_N() {
   unsigned r = my_random(14495618);
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count in bucket, second is bucket upper limit
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
      if (r < br.first) { /// # < br -> interpolate
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldn't happen, passed last break 2 " << std::to_string(r) << std::endl;
   return 1;
}

unsigned gen_N_len() {
   unsigned r = my_random(77869427);
   std::vector<std::pair<unsigned, unsigned>> breaks = /// histogram, first is count in bucket, second is bucket upper limit
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
      if (r < br.first) { /// # < br -> interpolate
         return (double) (r - last_br.first) / (double) (br.first - last_br.first) * (br.second - last_br.second);
      }
   }
   std::cerr << "Shouldn't happen, passed last break 3 " << std::to_string(r) << std::endl;
   return 1;
}

char sample_pos(std::vector<uint32_t>& dist) {
   if (dist.back() == 0) {
      std::cerr << "dist back is 0" << std::endl;
      return 'N';
   }
   unsigned r = my_random(dist.back());
   for (unsigned symbol = 0; symbol < dist.size(); ++symbol) {
      if (r < dist[symbol]) {
         return silo::symbol_rep[symbol];
      }
   }
   std::cerr << "Shouldn't happen, passed last break 4 " << std::to_string(r) << " " << std::to_string(dist.back()) << std::endl;
   return 'A';
}

void gen_genome(char* ret, std::vector<std::vector<uint32_t>>& dist) {
   unsigned start_N = gen_start_N();
   unsigned end_N = gen_end_N();
   unsigned num_inner_N_runs = my_random(12);

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
      unsigned offset = (my_random(inner_len - N_run_length)) + start_N + 1;
      for (unsigned i = offset; i < offset + N_run_length; ++i) {
         ret[i] = 'N';
      }
   }
}

int bootstrap(const silo::Database& db, std::string& out_dir, unsigned seed, unsigned factor) {
   unsigned pango_count = db.dict->get_pango_count();
   unsigned epi_factor = pango_count * factor;
   tbb::parallel_for(tbb::blocked_range<unsigned>(0, pango_count), [&](tbb::blocked_range<unsigned> local) {
      for (auto pango_id = local.begin(); pango_id < local.end(); pango_id++) {
         lehmer64_seed(seed * pango_id);
         auto predicate = std::make_unique<silo::PangoLineageEx>(pango_id, false);
         auto part_filters = silo::execute_predicate(db, predicate.get());
         unsigned count = 0;
         for (auto& f : part_filters) count += f.getAsConst()->cardinality();
         auto dist = silo::execute_all_dist(db, part_filters);
         char genome[silo::genomeLength]; // Buffer
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
                  out_file << "EPI_ISL_" << std::to_string(i * epi_factor + rep * pango_count + pango_id) << '\n';
                  gen_genome(genome, dist);
                  out_file << genome << '\n';
               }
            }
            /// xz compress output file
            {
               std::string compress_command = "xz -T0 -z " + file_name;
               int ret_code = system(compress_command.c_str());
               if (ret_code) {
                  std::cerr << "Command to compress generated file " << file_name << ":\n"
                            << compress_command << '\n'
                            << "Returned with non zero return code " << ret_code << std::endl;
               }
            }
         }
      }
      return 0;
   });
   return 0;
}
