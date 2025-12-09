# python/pysilo/column_type.pyx
from enum import IntEnum
from column_type cimport ColumnType as CppColumnType

# Extract values to module-level variables
_STRING_VAL = <int>CppColumnType.STRING
_INDEXED_STRING_VAL = <int>CppColumnType.INDEXED_STRING
_DATE_VAL = <int>CppColumnType.DATE
_BOOL_VAL = <int>CppColumnType.BOOL
_INT32_VAL = <int>CppColumnType.INT32
_FLOAT_VAL = <int>CppColumnType.FLOAT
_AMINO_ACID_SEQUENCE_VAL = <int>CppColumnType.AMINO_ACID_SEQUENCE
_NUCLEOTIDE_SEQUENCE_VAL = <int>CppColumnType.NUCLEOTIDE_SEQUENCE
_ZSTD_COMPRESSED_STRING_VAL = <int>CppColumnType.ZSTD_COMPRESSED_STRING
_INT64_VAL = <int>CppColumnType.INT64

# Now define the enum using those values
class PyColumnType(IntEnum):
    """Column type enumeration"""
    STRING = _STRING_VAL
    INDEXED_STRING = _INDEXED_STRING_VAL
    DATE = _DATE_VAL
    BOOL = _BOOL_VAL
    INT32 = _INT32_VAL
    FLOAT = _FLOAT_VAL
    AMINO_ACID_SEQUENCE = _AMINO_ACID_SEQUENCE_VAL
    NUCLEOTIDE_SEQUENCE = _NUCLEOTIDE_SEQUENCE_VAL
    ZSTD_COMPRESSED_STRING = _ZSTD_COMPRESSED_STRING_VAL
    INT64 = _INT64_VAL

# Helper functions - these go in .pyx not .pxd!
cdef CppColumnType to_cpp_column_type(column_type):
    """Convert Python ColumnType to C++ ColumnType"""
    if isinstance(column_type, int):
        return <CppColumnType>column_type
    elif isinstance(column_type, PyColumnType):
        return <CppColumnType>column_type.value
    else:
        raise TypeError(f"Expected ColumnType or int, got {type(column_type)}")

cdef object from_cpp_column_type(CppColumnType cpp_type):
    """Convert C++ ColumnType to Python ColumnType"""
    return PyColumnType(<int>cpp_type)
