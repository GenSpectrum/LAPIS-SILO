
#include <readline/history.h>
#include <readline/readline.h>
#include <silo/benchmark.h>
#include <silo/bootstrap.h>
#include <silo/common/istream_wrapper.h>
#include <silo/database.h>
#include <silo/prepare_dataset.h>
#include <silo/query_engine/query_engine.h>
#include <silo/common/PerfEvent.hpp>

using namespace silo;

void info_message() {
   using std::cin;
   using std::cout;
   using std::endl;
   cout << "SILO - Sequence Indexing engine for Large genOmic data" << endl << endl;
   cout << "Usage:" << endl;
   cout << "\tsilo" << endl;
   cout << "\tStart silo in interactive mode" << endl << endl;
   cout << "\tsilo \"<command>\" ..." << endl;
   cout << "\tExecute the commands in the given order, then enter interactive mode." << endl
        << endl;
   cout << "\tPreprocessing commands:" << endl
        << "\trepair_meta [metadata_file] [sequence_file] [meta_out]" << endl
        << "\trepair_sequences [metadata_file] [sequence_file] [sequences_out]" << endl
        << "\tbuild_pango_def [metadata_file]" << endl
        << "\tbuild_part_def [Partition mode: 1=all chunks, 2=all partitions, 3=single single]"
        << endl
        << "\tpartition [metadata_file] [sequence_file] [partition_directory]" << endl
        << "\tsort_chunks [partition_directory]" << endl
        << endl
        << "\tDatabase building commands:" << endl
        << "\tbuild_dict [partition_directory] [meta_suffix]" << endl
        << "\tbuild [partition_directory] [meta_suffix] [fasta_suffix] [info out]" << endl
        << "\tsave [save_directory]" << endl
        << "\tload [load_directory]" << endl
        << "\tflip_bitmaps" << endl
        << "\trun_optimize" << endl
        << "\tremove_run_optimize" << endl
        << "\tshrink_to_fit" << endl
        << "\tindex_all_n" << endl
        << "\tindex_all_n_naive" << endl
        << endl
        << "\tAnalytics commands:" << endl
        << "\tinfo [outfile]" << endl
        << "\tinfo_d [outfile]" << endl
        << "\tprint_flipped [outfile]" << endl
        << "\tbenchmark [query_dir]" << endl
        << "\tbenchmark_throughput [query_dir]" << endl
        << "\tquery [query_name] [query_dir]" << endl;
}

int handle_command(Database& db, std::vector<std::string> args) {
   using std::endl;

   const std::string default_db_savedir = db.wd + "bin_save/";
   const std::string default_sequence_input = db.wd + "minimal_sequence_set.fasta";
   const std::string default_metadata_input = db.wd + "minimal_metadata_set.tsv";
   const std::string default_partition_prefix = db.wd + "Partitioned/";
   const std::string default_pango_def_file = db.wd + "pango_descriptor.txt";
   const std::string default_part_def_file = db.wd + "partition_descriptor.txt";
   const std::string default_dict_file = db.wd + "dict.txt";
   const std::string default_query_dir = db.wd + "queries/";
   if (args.empty()) {
      return 0;
   }
   if ("repair_meta" == args[0]) {
      auto meta_input_str = args.size() > 1 ? args[1] : default_metadata_input;
      auto meta_input = std::ifstream(meta_input_str);
      if (!meta_input) {
         std::cerr << "metadata file " << (meta_input_str) << " not found." << std::endl;
         return 0;
      }
      auto sequence_input_str = args.size() > 2 ? args[2] : default_sequence_input;
      auto sequence_input = silo::istream_wrapper(sequence_input_str);
      if (!sequence_input.get_is()) {
         std::cerr << "sequence file " << (sequence_input_str) << " not found." << std::endl;
         return 0;
      }
      auto meta_out = args.size() > 3 ? std::ofstream(args[3])
                                      : std::ofstream(default_metadata_input + ".repair");
      prune_meta(meta_input, sequence_input.get_is(), meta_out);
   } else if ("repair_sequences" == args[0]) {
      auto meta_input_str = args.size() > 1 ? args[1] : default_metadata_input;
      auto meta_input = std::ifstream(meta_input_str);
      if (!meta_input) {
         std::cerr << "metadata file " << (meta_input_str) << " not found." << std::endl;
         return 0;
      }
      auto sequence_input_str = args.size() > 2 ? args[2] : default_sequence_input;
      auto sequence_input = silo::istream_wrapper(sequence_input_str);
      if (!sequence_input.get_is()) {
         std::cerr << "sequence file " << (sequence_input_str) << " not found." << std::endl;
         return 0;
      }
      auto sequence_out = args.size() > 3 ? std::ofstream(args[3])
                                          : std::ofstream(default_sequence_input + ".repair");
      prune_sequences(meta_input, sequence_input.get_is(), sequence_out);
   } else if ("info_d" == args[0]) {
      if (args.size() > 1) {
         std::ofstream out(args[1]);
         if (!out) {
            std::cout << "Could not open outfile " << args[1] << endl;
            return 0;
         }
         db.db_info_detailed(out);
      } else {
         db.db_info_detailed(std::cout);
      }
   } else if ("print_flipped" == args[0]) {
      if (args.size() > 1) {
         std::ofstream out(args[1]);
         if (!out) {
            std::cout << "Could not open outfile " << args[1] << endl;
            return 0;
         }
         db.print_flipped(out);
      } else {
         db.print_flipped(std::cout);
      }
   } else if ("load" == args[0]) {
      std::string db_savedir = args.size() > 1 ? args[1] : default_db_savedir;
      std::cout << "Loading Database from " << db_savedir << endl;
      db.load(db_savedir);
   } else if ("save" == args[0]) {
      std::string db_savedir = args.size() > 1 ? args[1] : default_db_savedir;
      std::cout << "Saving Database to " << db_savedir << endl;
      db.save(db_savedir);
   } else if ("benchmark" == args[0]) {
      auto query_dir_str = args.size() > 1 ? args[1] : default_query_dir;

      auto query_defs = std::ifstream(query_dir_str + "queries.txt");
      if (!query_defs) {
         std::cerr << "query_defs file " << (query_dir_str + "queries.txt") << " not found."
                   << std::endl;
         return 0;
      }
      return benchmark(db, query_defs, query_dir_str);
   } else if ("benchmark_throughput" == args[0]) {
      auto query_dir_str = args.size() > 1 ? args[1] : default_query_dir;

      auto query_defs = std::ifstream(query_dir_str + "queries.txt");
      if (!query_defs) {
         std::cerr << "query_defs file " << (query_dir_str + "queries.txt") << " not found."
                   << std::endl;
         return 0;
      }
      return benchmark_throughput(db, query_defs, query_dir_str);
   } else if ("benchmark_throughput_mix" == args[0]) {
      auto query_dir_str = args.size() > 1 ? args[1] : default_query_dir;

      auto query_defs = std::ifstream(query_dir_str + "queries.txt");
      if (!query_defs) {
         std::cerr << "query_defs file " << (query_dir_str + "queries.txt") << " not found."
                   << std::endl;
         return 0;
      }
      return benchmark_throughput_mix(db, query_defs, query_dir_str);
   } else if ("bootstrap" == args[0]) {
      if (args.size() <= 2) {
         std::cerr << "Need to specify output directory and seed for bootstrapping." << std::endl;
         return 0;
      }
      auto out_dir = args[1];
      auto seed = atoi(args[2].c_str());
      auto factor = args.size() > 3 ? atoi(args[3].c_str()) : 10;
      return bootstrap(db, out_dir, seed, factor);
   } else if ("benchmark_throughput_mut" == args[0]) {
      auto query_dir_str = args.size() > 1 ? args[1] : default_query_dir;

      auto query_defs = std::ifstream(query_dir_str + "queries.txt");
      if (!query_defs) {
         std::cerr << "query_defs file " << (query_dir_str + "queries.txt") << " not found."
                   << std::endl;
         return 0;
      }
      return benchmark_throughput_mut(db, query_defs, query_dir_str);
   } else if ("save_pango_def" == args[0]) {
      if (!db.pango_descriptor) {
         std::cout << "No pango_descriptor initialized. See 'build_pango_def' | 'load_pango_def'"
                   << std::endl;
         return 0;
      }
      auto pango_def_output_str = args.size() > 1 ? args[1] : default_pango_def_file;
      auto pango_def_output = std::ofstream(pango_def_output_str);
      std::cout << "Save pango_descriptor to file " << pango_def_output_str << std::endl;
      silo::save_pango_defs(*db.pango_descriptor, pango_def_output);
      return 0;
   } else if ("load_pango_def" == args[0]) {
      auto pango_def_input_str = args.size() > 1 ? args[1] : default_pango_def_file;
      auto pango_def_input = std::ifstream(pango_def_input_str);
      if (!pango_def_input) {
         std::cerr << "pango_def_input file " << pango_def_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Load pango_descriptor from input file " << pango_def_input_str << std::endl;
      db.pango_descriptor =
         std::make_unique<pango_descriptor_t>(silo::load_pango_defs(pango_def_input));
      return 0;
   } else if ("save_part_def" == args[0]) {
      if (!db.partition_descriptor) {
         std::cerr << "No partition_descriptor initialized. See 'build_part_def' | 'load_part_def'"
                   << std::endl;
         return 0;
      }
      auto part_def_output_str = args.size() > 1 ? args[1] : default_part_def_file;
      auto part_def_output = std::ofstream(part_def_output_str);
      std::cout << "Save partition_descriptor to file " << part_def_output_str << std::endl;
      silo::save_partitioning_descriptor(*db.partition_descriptor, part_def_output);
      return 0;
   } else if ("load_part_def" == args[0]) {
      auto part_def_input_str = args.size() > 1 ? args[1] : default_part_def_file;
      auto part_def_input = std::ifstream(part_def_input_str);
      if (!part_def_input) {
         std::cerr << "part_def_input file " << part_def_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Load partition_descriptor from input file " << part_def_input_str << std::endl;
      partitioning_descriptor_t part_def = silo::load_partitioning_descriptor(part_def_input);
      db.partition_descriptor = std::make_unique<partitioning_descriptor_t>(part_def);
      return 0;
   } else if ("save_dict" == args[0]) {
      if (!db.dict) {
         std::cerr << "Dict not initialized. See 'build_dict' | 'load_dict'" << std::endl;
         return 0;
      }
      auto dict_output_str = args.size() > 1 ? args[1] : default_dict_file;
      auto dict_output = std::ofstream(dict_output_str);
      if (!dict_output) {
         std::cerr << "Could not open '" << dict_output_str << "'." << std::endl;
         return 0;
      }
      std::cout << "Save dictionary to file " << dict_output_str << std::endl;
      db.dict->save_dict(dict_output);
      return 0;
   } else if ("load_dict" == args[0]) {
      auto dict_input_str = args.size() > 1 ? args[1] : default_dict_file;
      auto dict_input = std::ifstream(dict_input_str);
      if (!dict_input) {
         std::cerr << "dict_input file " << dict_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Load dictionary from input file " << dict_input_str << std::endl;
      db.dict = std::make_unique<Dictionary>(Dictionary::load_dict(dict_input));
      return 0;
   } else if ("flip_bitmaps" == args[0]) {
      db.flipBitmaps();
   } else if ("run_optimize" == args[0]) {
      uint32_t optimised = 0;
      for (auto& dbp : db.partitions) {
         optimised += runOptimize(dbp.seq_store);
      }
      uint32_t total_bitmaps = (genomeLength * Symbol::N * db.partitions.size());
      std::cout << "Optimised " << std::to_string(optimised) << " out of " << total_bitmaps
                << " bitmaps." << std::endl;
   } else if ("remove_run_optimize" == args[0]) {
      for (auto& dbp : db.partitions) {
         for (auto& position : dbp.seq_store.positions) {
            for (auto& bm : position.bitmaps) {
               bm.removeRunCompression();
            }
         }
      }
      std::cout << "Removed run compression." << std::endl;
   } else if ("shrink_to_fit" == args[0]) {
      size_t saved = 0;
      for (auto& dbp : db.partitions) {
         saved += shrinkToFit(dbp.seq_store);
      }
      std::cout << "Saved " << saved << " bytes by call to shrink_to_fit." << std::endl;
   } else if ("index_all_n" == args[0]) {
      db.indexAllN();
   } else if ("index_all_n_naive" == args[0]) {
      db.indexAllN_naive();
   } else if ("exit" == args[0] || "quit" == args[0]) {
      return 1;
   } else if ("help" == args[0] || "-h" == args[0] || "--help" == args[0]) {
      info_message();
   } else if ("gap_analysis" == args[0]) {
      std::ofstream out("../start_end_N_analysis.tsv");
      out << /*"EPI\t*/ "start_N\tend_N\n";

      istream_wrapper in(default_sequence_input);
      std::string epi, genome;
      while (true) {
         if (!getline(in.get_is(), epi, '\n'))
            break;
         if (!getline(in.get_is(), genome, '\n'))
            break;

         unsigned start_gaps = 0;
         while (start_gaps < genome.length() && genome.at(start_gaps) == 'N') {
            start_gaps++;
         }

         unsigned end_gaps = 0;
         while (end_gaps < genome.length() && genome.at(genome.length() - 1 - end_gaps) == 'N') {
            end_gaps++;
         }

         out << /*epi << "\t" <<*/ std::to_string(start_gaps) << "\t" << std::to_string(end_gaps)
             << "\n";
      }
      out.flush();
   } else if ("N_analysis" == args[0]) {
      std::ofstream out("../N_analysis.tsv");
      out << /*"EPI\tpos\t*/ "len\n";

      istream_wrapper in(default_sequence_input);
      std::string epi, genome;
      while (true) {
         if (!getline(in.get_is(), epi, '\n'))
            break;
         if (!getline(in.get_is(), genome, '\n'))
            break;

         unsigned idx = 0;
         while (idx < genomeLength) {
            while (genome.at(idx) != 'N') {
               idx++;
               if (idx == genomeLength)
                  break;
            }
            if (idx == genomeLength)
               break;
            unsigned N_start = idx;
            while (genome.at(idx) == 'N') {
               idx++;
               if (idx == genomeLength)
                  break;
            }
            out << /* epi << "\t" << std::to_string(N_start) << "\t" << */ std::to_string(
                      idx - N_start
                   )
                << "\n";
         }
      }
      out.flush();
   } else if ("inner_N_analysis" == args[0]) {
      std::ofstream out("../inner_N_analysis.tsv");
      out << /*"EPI\tpos\t*/ "len\n";

      istream_wrapper in(default_sequence_input);
      std::string epi, genome;
      while (true) {
         if (!getline(in.get_is(), epi, '\n'))
            break;
         if (!getline(in.get_is(), genome, '\n'))
            break;

         unsigned last_seen = 0;
         bool first = true;

         unsigned idx = 0;
         while (idx < genomeLength) {
            while (genome.at(idx) != 'N') {
               idx++;
               if (idx == genomeLength)
                  break;
            }
            if (idx == genomeLength)
               break;
            unsigned N_start = idx;
            while (genome.at(idx) == 'N') {
               idx++;
               if (idx == genomeLength)
                  break;
            }
            if (last_seen) {
               out << /* epi << "\t" << std::to_string(N_start) << "\t" << */ std::to_string(
                         last_seen
                      )
                   << "\n";
            }
            if (first) {
               first = false;
            } else {
               last_seen = idx - N_start;
            }
         }
      }
      out.flush();
   }
   // ___________________________________________________________________________________________________________
   else {
      std::cout << "Unknown command " << args[0] << "." << endl;
      std::cout << "Type 'help' for additional information." << endl;
      return 0;
   }
   return 0;
}

int handle_command(Database& db, const std::string& command_str) {
   std::stringstream ss(command_str);
   std::istream_iterator<std::string> begin(ss);
   std::istream_iterator<std::string> end;
   std::vector<std::string> command(begin, end);
   return handle_command(db, command);
}

int main(int argc, char* argv[]) {
   try {
      std::string wd = "./";
      std::vector<std::string> startup_commands;
      if (argc >= 2) {
         for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-w") {
               if (i + 1 < argc) {
                  wd = argv[i + 1];
                  i++;
               } else {
                  std::cerr << "-w option passed without wd parameter following it." << std::endl;
                  return 0;
               }
            } else if (arg.starts_with("-w=")) {
               wd = arg.substr(2);
            } else {
               // All parameters are fed to the database as inputs following the parsing of
               // parameters
               startup_commands.push_back(argv[i]);
            }
         }
      }

      auto db = std::make_unique<Database>(wd);

      for (auto& command : startup_commands) {
         // Stop execution if handle_command returns anything other than 0
         if (handle_command(*db, command) != 0) {
            return 0;
         }
      }

      char* buf;
      while ((buf = readline(">> ")) != nullptr) {
         if (strlen(buf) > 0) {
            add_history(buf);

            if (handle_command(*db, std::string(buf)) != 0) {
               return 0;
            }
         }
         // readline malloc's a new buffer every time.
         free(buf);
      }
   } catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
   }
   return 0;
}
