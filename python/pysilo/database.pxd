from libcpp.string cimport string
from libcpp.vector cimport vector
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier

cdef extern from "silo/database.h":
    cdef cppclass Database:
        Database() except +
        void createTable(string table_name, vector[CppColumnIdentifier] fields) except +
        void appendData(string table_name, string file_name) except +