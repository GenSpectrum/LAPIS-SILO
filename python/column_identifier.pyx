# column_identifier.pyx
from libcpp.string cimport string
from column_identifier cimport ColumnIdentifier as CppColumnIdentifier
from column_type cimport ColumnType as CppColumnType, to_cpp_column_type, from_cpp_column_type
from column_type import ColumnType  # Import Python enum class

cdef class ColumnIdentifier:
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
            Column type (can pass enum or integer value)

        Examples
        --------
        >>> col1 = ColumnIdentifier("age", ColumnType.INT32)
        >>> col2 = ColumnIdentifier("name", ColumnType.STRING)
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
        """Get the column type as a ColumnType enum"""
        return from_cpp_column_type(self.c_identifier.type)

    @type.setter
    def type(self, column_type):
        """Set the column type (accepts ColumnType enum or int)"""
        self.c_identifier.type = to_cpp_column_type(column_type)

    def __eq__(self, ColumnIdentifier other):
        """Check equality with another ColumnIdentifier"""
        if not isinstance(other, ColumnIdentifier):
            return False
        return self.c_identifier == other.c_identifier

    def __lt__(self, ColumnIdentifier other):
        """Check if less than another ColumnIdentifier"""
        return self.c_identifier < other.c_identifier

    def __hash__(self):
        """Make ColumnIdentifier hashable"""
        return hash((self.name, self.type))

    def __repr__(self):
        return f"ColumnIdentifier(name='{self.name}', type={self.type.name})"

# Conversion helpers
cdef CppColumnIdentifier to_cpp_column_identifier(ColumnIdentifier py_obj):
    """Convert Python ColumnIdentifier to C++ ColumnIdentifier"""
    return py_obj.c_identifier

cdef ColumnIdentifier from_cpp_column_identifier(const CppColumnIdentifier& cpp_obj):
    """Convert C++ ColumnIdentifier to Python ColumnIdentifier"""
    cdef ColumnIdentifier py_obj = ColumnIdentifier.__new__(ColumnIdentifier)
    py_obj.c_identifier = cpp_obj
    return py_obj
