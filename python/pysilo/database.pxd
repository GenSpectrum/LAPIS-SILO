from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.optional cimport optional
from libc.stdint cimport uint64_t, uint32_t

cdef extern from "exception_handler.h":
    void handle_silo_exception()

cdef extern from "roaring/roaring.hh" namespace "roaring":
    cdef cppclass Roaring:
        Roaring() except +
        size_t getSizeInBytes() except +
        size_t write(char* buf) except +

cdef extern from "silo/database.h" namespace "silo":
    cdef cppclass Database:
        Database() except +
        Database(const Database&) except +  # Copy constructor
        void createNucleotideSequenceTable(string table_name, string primary_key_name, string sequence_name, string reference_sequence, vector[string] extra_string_columns) except +
        void createGeneTable(string table_name, string primary_key_name, string sequence_name, string reference_sequence, vector[string] extra_string_columns) except +
        void appendDataFromFile(string table_name, string file_name) except +
        void appendDataFromString(string table_name, string json_string) except +
        void printAllData(string table_name) except +
        string getNucleotideReferenceSequence(string table_name, string sequence_name) except +
        string getAminoAcidReferenceSequence(string table_name, string sequence_name) except +
        vector[pair[uint64_t, string]] getPrevalentNucMutations(string table_name, string sequence_name, double prevalence_threshold, string filter) except +handle_silo_exception
        vector[pair[uint64_t, string]] getPrevalentAminoAcidMutations(string table_name, string sequence_name, double prevalence_threshold, string filter) except +handle_silo_exception
        Roaring getFilteredBitmap(string table_name, string filter) except +handle_silo_exception
        void saveDatabaseState(string save_directory) except +
        string executeQueryAsArrowIpc(string table_name, string query_json) except +handle_silo_exception
        string getTablesAsArrowIpc() except +

        @staticmethod
        optional[Database] loadDatabaseStateFromPath(string save_directory) except +