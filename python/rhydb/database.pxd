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

cdef extern from "rhydb/database.h" namespace "rhydb":
    cdef struct ColumnDefinition:
        string name
        string type

    cdef struct ReferenceEntry:
        string name
        string reference
        string type

    cdef cppclass Database:
        Database() except +
        Database(const Database&) except +  # Copy constructor
        void createTableFromColumns(string table_name, vector[ColumnDefinition] columns) except +
        void addReferences(const vector[ReferenceEntry]& entries) except +
        void appendDataFromFile(string table_name, string file_name) except +
        void appendDataFromString(string table_name, string json_string) except +
        void printAllData(string table_name) except +
        string getNucleotideReferenceSequence(string table_name, string sequence_name) except +
        string getAminoAcidReferenceSequence(string table_name, string sequence_name) except +
        Roaring getFilteredBitmap(string table_name, string filter) except +handle_silo_exception
        void updateColumn(string table_name, string column_name, string value, string filter_expression) except +handle_silo_exception
        void saveDatabaseState(string save_directory) except +
        string executeQueryAsArrowIpc(string query_string) except +handle_silo_exception
        string getTablesAsArrowIpc() except +

        @staticmethod
        optional[Database] loadDatabaseStateFromPath(string save_directory) except +