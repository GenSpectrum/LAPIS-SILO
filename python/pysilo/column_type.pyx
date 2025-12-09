# python/pysilo/column_type.pyx
from enum import IntEnum
from column_type cimport ColumnType as CppColumnType

class ColumnType(IntEnum):
    """Column type enumeration"""
    STRING = <int>CppColumnType.STRING
    INDEXED_STRING = <int>CppColumnType.INDEXED_STRING
    DATE = <int>CppColumnType.DATE
    BOOL = <int>CppColumnType.BOOL
    INT32 = <int>CppColumnType.INT32
    FLOAT = <int>CppColumnType.FLOAT
    AMINO_ACID_SEQUENCE = <int>CppColumnType.AMINO_ACID_SEQUENCE
    NUCLEOTIDE_SEQUENCE = <int>CppColumnType.NUCLEOTIDE_SEQUENCE
    ZSTD_COMPRESSED_STRING = <int>CppColumnType.ZSTD_COMPRESSED_STRING
    INT64 = <int>CppColumnType.INT64

# Helper functions - these go in .pyx not .pxd!
cdef CppColumnType to_cpp_column_type(column_type):
    """Convert Python ColumnType to C++ ColumnType"""
    if isinstance(column_type, int):
        return <CppColumnType>column_type
    elif isinstance(column_type, ColumnType):
        return <CppColumnType>column_type.value
    else:
        raise TypeError(f"Expected ColumnType or int, got {type(column_type)}")

cdef object from_cpp_column_type(CppColumnType cpp_type):
    """Convert C++ ColumnType to Python ColumnType"""
    return ColumnType(<int>cpp_type)
