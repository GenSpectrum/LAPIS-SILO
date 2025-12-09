from libcpp.string cimport string
from libcpp.vector cimport vector
from database cimport Database as CppDatabase
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier
from column_identifier import ColumnIdentifier
from typing import List

cdef class Database:
    """
    Python wrapper for the C++ Database class.
    
    Provides methods to create tables and append data.
    """
    cdef CppDatabase* c_database  # Pointer to the C++ Database object
    
    def __cinit__(self):
        """Initialize the C++ Database object"""
        self.c_database = new CppDatabase()
    
    def __dealloc__(self):
        """Clean up the C++ Database object"""
        if self.c_database != NULL:
            del self.c_database
    
    def create_table(self, str table_name, list fields):
        """
        Create a new table with specified columns.
        
        Parameters
        ----------
        table_name : str
            Name of the table to create
        fields : list of ColumnIdentifier
            List of column identifiers defining the table schema
        
        Examples
        --------
        >>> from your_module import Database, ColumnIdentifier, ColumnType
        >>> db = Database()
        >>> fields = [
        ...     ColumnIdentifier("id", ColumnType.INT32),
        ...     ColumnIdentifier("name", ColumnType.STRING),
        ...     ColumnIdentifier("age", ColumnType.INT32)
        ... ]
        >>> db.create_table("users", fields)
        """
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef vector[CppColumnIdentifier] cpp_fields
        cdef ColumnIdentifier field
        
        # Convert Python list to C++ vector
        for field in fields:
            if not isinstance(field, ColumnIdentifier):
                raise TypeError(f"Expected ColumnIdentifier, got {type(field)}")
            cpp_fields.push_back(field.c_identifier)
        
        # Call the C++ method
        self.c_database.createTable(cpp_table_name, cpp_fields)
    
    def append_data(self, str table_name, str file_name):
        """
        Append data from a file to an existing table.
        
        Parameters
        ----------
        table_name : str
            Name of the table to append data to
        file_name : str
            Path to the file containing data to append
        
        Examples
        --------
        >>> db = Database()
        >>> db.append_data("users", "/path/to/data.csv")
        """
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_file_name = file_name.encode('utf-8')
        
        # Call the C++ method
        self.c_database.appendData(cpp_table_name, cpp_file_name)
    
    def __repr__(self):
        return "Database()"
