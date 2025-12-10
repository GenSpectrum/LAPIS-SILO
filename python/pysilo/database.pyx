# python/pysilo/database.pyx
from libcpp.string cimport string
from libcpp.vector cimport vector
from database cimport Database as CppDatabase
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier
from column_identifier cimport PyColumnIdentifier
import os

cdef class PyDatabase:
    """Python wrapper for C++ Database"""
    cdef CppDatabase* c_database
    
    def __cinit__(self):
        self.c_database = new CppDatabase()
    
    def __dealloc__(self):
        if self.c_database != NULL:
            del self.c_database
    
    def create_table(self, str table_name, list fields):
        """
        Create a new table
        
        Parameters
        ----------
        table_name : str
            Name of the table
        fields : list of ColumnIdentifier
            List of column identifiers
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        
        if not fields:
            raise ValueError("fields list cannot be empty")
        
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef vector[CppColumnIdentifier] cpp_fields
        cdef PyColumnIdentifier field
        
        for i, field in enumerate(fields):
            if not isinstance(field, PyColumnIdentifier):
                raise TypeError(
                    f"Field at index {i} must be ColumnIdentifier, "
                    f"got {type(field).__name__}"
                )
            cpp_fields.push_back(field.c_identifier)
        
        try:
            self.c_database.createSimpleTable(cpp_table_name, cpp_fields)
        except Exception as e:
            raise RuntimeError(f"Failed to create table '{table_name}': {e}")
    
    def append_data(self, str table_name, str file_name):
        """
        Append data from file to table
        
        Parameters
        ----------
        table_name : str
            Name of the table
        file_name : str
            Path to data file
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        
        if not file_name or not file_name.strip():
            raise ValueError("file_name cannot be empty")
        
        if not os.path.exists(file_name):
            raise FileNotFoundError(f"File not found: {file_name}")
        
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_file_name = file_name.encode('utf-8')
        
        try:
            self.c_database.appendData(cpp_table_name, cpp_file_name)
        except Exception as e:
            raise RuntimeError(f"Failed to append data: {e}")
    
    def __repr__(self):
        return "Database()"
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        return False