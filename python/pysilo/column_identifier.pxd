# column_identifier.pxd
from libcpp.string cimport string
from column_type cimport ColumnType  # Import the C++ enum

cdef extern from "silo/schema/database_schema.h" namespace "silo::schema":
    cdef cppclass ColumnIdentifier:
        ColumnIdentifier() except +
        ColumnIdentifier(string name, ColumnType type) except +
        string name
        ColumnType type
        bint operator<(const ColumnIdentifier& other)
        bint operator==(const ColumnIdentifier& other)
