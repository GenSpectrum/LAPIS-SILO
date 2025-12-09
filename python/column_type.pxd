cdef extern from "silo/schema/column_type.h" namespace "silo::schema":
    cdef enum class ColumnType(unsigned char):  # uint8_t = unsigned char
        STRING
        INDEXED_STRING
        DATE
        BOOL
        INT32
        FLOAT
        AMINO_ACID_SEQUENCE
        NUCLEOTIDE_SEQUENCE
        ZSTD_COMPRESSED_STRING
        INT64
