# python/pysilo/column_identifier.pyx
from libcpp.string cimport string
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier
from column_type cimport ColumnType as CppColumnType
from . import column_type

# Inline helper functions
cdef CppColumnType py_to_cpp_column_type(column_type):
    """Convert Python ColumnType to C++ ColumnType"""
    if isinstance(column_type, int):
        return <CppColumnType>column_type
    elif isinstance(column_type, column_type.PyColumnType):
        return <CppColumnType>column_type.value
    else:
        raise TypeError(f"Expected ColumnType or int, got {type(column_type)}")

cdef object cpp_to_py_column_type(CppColumnType cpp_type):
    """Convert C++ ColumnType to Python ColumnType"""
    return column_type.PyColumnType(<int>cpp_type)

# Python wrapper class - use a different name!
cdef class PyColumnIdentifier:
    """Python wrapper for C++ ColumnIdentifier"""

    def __init__(self, str name, col_type):
        """
        Create a ColumnIdentifier
        
        Parameters
        ----------
        name : str
            Column name
        col_type : ColumnType or int
            Column type
        """
        cdef string cpp_name = name.encode('utf-8')
        cdef CppColumnType cpp_type = py_to_cpp_column_type(col_type)
        self.c_identifier = CppColumnIdentifier(cpp_name, cpp_type)
    
    @property
    def name(self) -> str:
        """Get the column name"""
        return self.c_identifier.name.decode('utf-8')
    
    @name.setter
    def name(self, str value):
        """Set the column name"""
        self.c_identifier.name = value.encode('utf-8')
    
    @property
    def type(self):
        """Get the column type"""
        return cpp_to_py_column_type(self.c_identifier.type)
    
    @type.setter
    def type(self, col_type):
        """Set the column type"""
        self.c_identifier.type = py_to_cpp_column_type(col_type)
    
    def __eq__(self, PyColumnIdentifier other):
        """Check equality"""
        if not isinstance(other, PyColumnIdentifier):
            return False
        return self.c_identifier == other.c_identifier
    
    def __lt__(self, PyColumnIdentifier other):
        """Check if less than"""
        return self.c_identifier < other.c_identifier
    
    def __hash__(self):
        """Make hashable"""
        return hash((self.name, self.type))
    
    def __repr__(self):
        return f"ColumnIdentifier(name='{self.name}', type={self.type.name})"