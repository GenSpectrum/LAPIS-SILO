from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "silo/database.h" namespace "silo":
    cdef cppclass Database:
        Database() except +
        void createNucleotideSequenceTable(string table_name, string primary_key_name, string sequence_name, string reference_sequence) except +
        void appendData(string table_name, string file_name) except +