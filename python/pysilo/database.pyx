# python/pysilo/database.pyx
from libcpp.string cimport string
from libcpp.vector cimport vector
from database cimport Database as CppDatabase
import os

cdef class PyDatabase:
    """Python wrapper for C++ Database"""
    cdef CppDatabase* c_database
    
    def __cinit__(self):
        self.c_database = new CppDatabase()
    
    def __dealloc__(self):
        if self.c_database != NULL:
            del self.c_database
    
    def create_table(self, str table_name, str primary_key_name, str sequence_name, str reference_sequence):
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
        if not primary_key_name or not primary_key_name.strip():
            raise ValueError("primary_key_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")
        if not reference_sequence or not reference_sequence.strip():
            raise ValueError("reference_sequence cannot be empty")
        
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_primary_key_name = primary_key_name.encode('utf-8')
        cdef string cpp_sequence_name = sequence_name.encode('utf-8')
        cdef string cpp_reference_sequence = reference_sequence.encode('utf-8')
        
        try:
            self.c_database.createNucleotideSequenceTable(cpp_table_name, cpp_primary_key_name, cpp_sequence_name, cpp_reference_sequence)
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