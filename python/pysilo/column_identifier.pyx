# python/pysilo/column_identifier.pyx
from libcpp.string cimport string
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier
from column_type cimport ColumnType as CppColumnType
from column_type import ColumnType
# Import the helper functions from column_type.pyx
from column_type cimport to_cpp_column_type, from_cpp_column_type

cdef class PyColumnIdentifier:
    """Python wrapper for C++ ColumnIdentifier"""
    cdef CppColumnIdentifier c_identifier
    
    def __init__(self, str name, column_type):
        """
        Create a ColumnIdentifier
        
        Parameters
        ----------
        name : str
            Column name
        column_type : ColumnType or int
            Column type
        """
        cdef string cpp_name = name.encode('utf-8')
        cdef CppColumnType cpp_type = to_cpp_column_type(column_type)
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
    def type(self) -> ColumnType:
        """Get the column type"""
        return from_cpp_column_type(self.c_identifier.type)
    
    @type.setter
    def type(self, column_type):
        """Set the column type"""
        self.c_identifier.type = to_cpp_column_type(column_type)
    
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

# Alias for convenience
ColumnIdentifier = PyColumnIdentifier