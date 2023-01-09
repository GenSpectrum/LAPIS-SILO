
#include <silo/common/PerfEvent.hpp>
#include <readline/history.h>
#include <readline/readline.h>
#include <silo/benchmark.h>
#include <silo/common/istream_wrapper.h>
#include <silo/database.h>
#include <silo/prepare_dataset.h>
#include <silo/query_engine/query_engine.h>

using namespace silo;

void info_message() {
   using std::cin;
   using std::cout;
   using std::endl;
   cout << "SILO - Sequence Indexing engine for Large genOmic data" << endl
        << endl;
   cout << "Usage:" << endl;
   cout << "\tsilo" << endl;
   cout << "\tStart silo in interactive mode" << endl
        << endl;
   cout << "\tsilo \"<command>\" ..." << endl;
   cout << "\tExecute the commands in the given order, then enter interactive mode." << endl
        << endl;
   cout << "\tCommands:" << endl
        << "\tbuild [fasta_archive]" << endl
        << "\tbuild_meta [metadata.tsv]" << endl
        << "\tpartition <out_prefix> [fasta_archive]" << endl
        << "\tsort_chunks <io_prefix> [fasta_archive]" << endl
        << "\tbuild_chunked [part_prefix] [part_suffix]" << endl;
}

int handle_command(Database& db, std::vector<std::string> args) {
   using std::cin;
   using std::cout;
   using std::endl;

   const std::string default_db_savedir = db.wd + "bin_save/";
   const std::string default_sequence_input = db.wd + "aligned.fasta.xz";
   const std::string default_metadata_input = db.wd + "metadata.tsv";
   const std::string default_partition_prefix = db.wd + "Partitioned/";
   const std::string default_pango_def_file = db.wd + "pango_def.txt";
   const std::string default_part_def_file = db.wd + "part_def.txt";
   const std::string default_dict_file = db.wd + "dict.txt";
   const std::string default_query_dir = db.wd + "queries/";
   if (args.empty()) {
      return 0;
   }
   if ("repair_meta" == args[0]) {
      auto meta_input = args.size() > 1 ? std::ifstream(args[1]) : std::ifstream(default_metadata_input);
      auto sequence_input = args.size() > 2 ? silo::istream_wrapper(args[2]) : silo::istream_wrapper(default_sequence_input);
      auto meta_out = args.size() > 3 ? std::ofstream(args[3]) : std::ofstream(default_metadata_input + ".repair");
      prune_meta(meta_input, sequence_input.get_is(), meta_out);
   } else if ("repair_sequences" == args[0]) {
      auto meta_input = args.size() > 1 ? std::ifstream(args[1]) : std::ifstream(default_metadata_input);
      auto sequence_input = args.size() > 2 ? silo::istream_wrapper(args[2]) : silo::istream_wrapper(default_sequence_input);
      auto sequence_out = args.size() > 3 ? std::ofstream(args[3]) : std::ofstream(default_sequence_input + ".repair");
      prune_sequences(meta_input, sequence_input.get_is(), sequence_out);
   } else if ("load" == args[0]) {
      std::string db_savedir = args.size() > 1 ? args[1] : default_db_savedir;
      cout << "Loading Database from " << db_savedir << endl;
      db.load(db_savedir);
   } else if ("save" == args[0]) {
      std::string db_savedir = args.size() > 1 ? args[1] : default_db_savedir;
      cout << "Saving Database to " << db_savedir << endl;
      db.save(db_savedir);
   } else if ("info" == args[0]) {
      db.db_info(cout);
   } else if ("info_d" == args[0]) {
      db.db_info_detailed(cout);
   } else if ("benchmark" == args[0]) {
      auto query_dir_str = args.size() > 1 ? args[1] : default_query_dir;

      auto query_defs = std::ifstream(query_dir_str + "queries.txt");
      if (!query_defs) {
         std::cerr << "query_defs file " << (query_dir_str + "queries.txt") << " not found." << std::endl;
         return 0;
      }
      return benchmark(db, query_defs, query_dir_str);
   } else if ("build_pango_def" == args[0]) {
      auto meta_input_str = args.size() > 1 ? args[1] : default_metadata_input;
      auto meta_input = std::ifstream(meta_input_str);
      if (!meta_input) {
         std::cerr << "meta_input file " << meta_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Build pango_def from file " << meta_input_str << std::endl;
      db.pango_def = std::make_unique<pango_descriptor_t>(silo::build_pango_defs(db.get_alias_key(), meta_input));
      return 0;
   } else if ("save_pango_def" == args[0]) {
      if (!db.pango_def) {
         std::cout << "No pango_def initialized. See 'build_pango_def' | 'load_pango_def'" << std::endl;
         return 0;
      }
      auto pango_def_output_str = args.size() > 1 ? args[1] : default_pango_def_file;
      auto pango_def_output = std::ofstream(pango_def_output_str);
      std::cout << "Save pango_def to file " << pango_def_output_str << std::endl;
      silo::save_pango_defs(*db.pango_def, pango_def_output);
      return 0;
   } else if ("load_pango_def" == args[0]) {
      auto pango_def_input_str = args.size() > 1 ? args[1] : default_pango_def_file;
      auto pango_def_input = std::ifstream(pango_def_input_str);
      if (!pango_def_input) {
         std::cerr << "pango_def_input file " << pango_def_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Load pango_def from input file " << pango_def_input_str << std::endl;
      db.pango_def = std::make_unique<pango_descriptor_t>(silo::load_pango_defs(pango_def_input));
      return 0;
   } else if ("build_part_def" == args[0]) {
      if (!db.pango_def) {
         std::cerr << "No pango_def initialized. See 'build_pango_def' | 'load_pango_def'" << std::endl;
         return 0;
      }
      std::cout << "Build part_def from pango_def" << std::endl;
      architecture_type arch = args.size() <= 1 || args[1] == "2" ? architecture_type::max_partitions :
         args[1] == "1"                                           ? architecture_type::single_partition :
                                                                    architecture_type::hybrid;
      partitioning_descriptor_t part_def = silo::build_partitioning_descriptor(*db.pango_def, arch);
      db.part_def = std::make_unique<partitioning_descriptor_t>(part_def);
      return 0;
   } else if ("save_part_def" == args[0]) {
      if (!db.part_def) {
         std::cerr << "No part_def initialized. See 'build_part_def' | 'load_part_def'" << std::endl;
         return 0;
      }
      auto part_def_output_str = args.size() > 1 ? args[1] : default_part_def_file;
      auto part_def_output = std::ofstream(part_def_output_str);
      std::cout << "Save part_def to file " << part_def_output_str << std::endl;
      silo::save_partitioning_descriptor(*db.part_def, part_def_output);
      return 0;
   } else if ("load_part_def" == args[0]) {
      auto part_def_input_str = args.size() > 1 ? args[1] : default_part_def_file;
      auto part_def_input = std::ifstream(part_def_input_str);
      if (!part_def_input) {
         std::cerr << "part_def_input file " << part_def_input_str << " not found." << std::endl;
         return 0;
      }
      std::cout << "Load part_def from input file " << part_def_input_str << std::endl;
      partitioning_descriptor_t part_def = silo::load_partitioning_descriptor(part_def_input);
      db.part_def = std::make_unique<partitioning_descriptor_t>(part_def);
      return 0;
   } else if ("partition" == args[0]) {
      if (!db.part_def) {
         cout << "Build partitioning descriptor first. See 'build_part_def' | 'load_part_def'" << endl;
         return 0;
      }
      silo::partitioning_descriptor_t& partitioning_descripter = *db.part_def;
      std::string meta_input = args.size() > 1 ? args[1] : default_metadata_input;
      std::string sequence_input = args.size() > 2 ? args[2] : default_sequence_input;
      std::string part_prefix = args.size() > 3 ? args[3] : default_partition_prefix;
      cout << "partition from " << sequence_input << " and " << meta_input << " into " << part_prefix << endl;
      istream_wrapper seq_file(sequence_input);
      if (!seq_file.get_is()) {
         std::cerr << "sequence_input file " << sequence_input << " not found." << std::endl;
         return 0;
      }
      std::ifstream meta_file(meta_input);
      if (!meta_file) {
         std::cerr << "meta_input file " << meta_input << " not found." << std::endl;
         return 0;
      }

      partition_sequences(partitioning_descripter, meta_file, seq_file.get_is(), part_prefix, db.get_alias_key());
      return 0;
   } else if ("sort_chunks" == args[0]) {
      if (!db.part_def) {
         cout << "Build partitioning descriptor first. See 'build_part_def' | 'load_part_def'" << endl;
         return 0;
      }
      std::string part_prefix = args.size() > 1 ? args[1] : default_partition_prefix;
      cout << "sort_chunks in " << part_prefix << endl;
      silo::sort_chunks(*db.part_def, part_prefix);
      return 0;
   } else if ("build_dict" == args[0]) {
      if (!db.part_def) {
         std::cerr << "No part_def initialized. See 'build_part_def' | 'load_part_def'" << std::endl;
         return 0;
      }
      std::string part_prefix = args.size() > 1 ? args[1] : default_partition_prefix;
      std::string meta_suffix = args.size() > 2 ? args[2] : ".meta.tsv";
      std::cout << "Build dictionary from meta_data in " << part_prefix << std::endl;
      db.dict = std::make_unique<Dictionary>();

      for (size_t i = 0; i < db.part_def->partitions.size(); ++i) {
         const auto& part = db.part_def->partitions[i];
         for (unsigned j = 0; j < part.chunks.size(); ++j) {
            std::string name;
            name = part_prefix + chunk_string(i, j);
            std::ifstream meta_in(name + meta_suffix);
            if (!meta_in) {
               std::cerr << "Meta_data file " << (name + meta_suffix) << " not found." << std::endl;
               return 0;
            }
            db.dict->update_dict(meta_in, db.get_alias_key());
         }
      }
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
   } else if ("build" == args[0]) {
      if (!db.part_def) {
         cout << "Build partitioning descriptor first. See 'build_part_def' | 'load_part_def'" << endl;
         return 0;
      }
      if (!db.dict) {
         cout << "Build dictionary first. See 'build_dict' | 'load_dict'" << endl;
         return 0;
      }
      std::string part_prefix = args.size() > 1 ? args[1] : default_partition_prefix;
      std::string meta_suffix = args.size() > 2 ? args[2] : ".meta.tsv";
      std::string seq_suffix = args.size() > 3 ? args[3] : ".fasta";
      db.build(part_prefix, meta_suffix, seq_suffix);
   } else if ("experiment" == args[0]) {
      db.finalize();
   } else if ("query" == args[0]) {
      if (args.size() < 2) {
         cout << "Expected syntax: \"query JSON_QUERY\"" << endl;
         return 0;
      }
      std::cerr << "TODO" << std::endl;
      /*
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
      cout << execute_query(db, mdb, query2.str()) << endl;*/
   } else if ("exit" == args[0] || "quit" == args[0]) {
      return 1;
   } else if ("help" == args[0] || "-h" == args[0] || "--help" == args[0]) {
      info_message();
   } else if ("gap_analysis" == args[0]) {
      std::ofstream out("../gap_analysis.tsv");
      out << "EPI\tstart_gaps\tend_gaps\n";

      istream_wrapper in(default_sequence_input);
      std::string epi, genome;
      while (true) {
         if (!getline(in.get_is(), epi, '\n')) break;
         if (!getline(in.get_is(), genome, '\n')) break;

         unsigned start_gaps = 0;
         while (start_gaps < genome.length() && genome.at(start_gaps) == '-') {
            start_gaps++;
         }

         unsigned end_gaps = 0;
         while (end_gaps < genome.length() && genome.at(genome.length() - 1 - end_gaps) == '-') {
            end_gaps++;
         }

         out << epi << "\t" << std::to_string(start_gaps) << "\t" << std::to_string(end_gaps) << "\n";
      }
      out.flush();
   } else if ("N_analysis" == args[0]) {
      std::ofstream out("../N_len_analysis.tsv");
      out << /*"EPI\tpos\t*/ "len\n";

      istream_wrapper in(default_sequence_input);
      std::string epi, genome;
      while (true) {
         if (!getline(in.get_is(), epi, '\n')) break;
         if (!getline(in.get_is(), genome, '\n')) break;

         unsigned idx = 0;
         while (idx < genomeLength) {
            while (genome.at(idx) != 'N') {
               idx++;
               if (idx == genomeLength) break;
            }
            if (idx == genomeLength) break;
            unsigned N_start = idx;
            while (genome.at(idx) == 'N') {
               idx++;
               if (idx == genomeLength) break;
            }
            out << /* epi << "\t" << std::to_string(N_start) << "\t" << */ std::to_string(idx - N_start) << "\n";
         }
      }
      out.flush();
   }
   // ___________________________________________________________________________________________________________
   else {
      cout << "Unknown command " << args[0] << "." << endl;
      cout << "Type 'help' for additional information." << endl;
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
               // All parameters are fed to the database as inputs following the parsing of parameters
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
