cdef extern from "silo/schema/database_schema.h" namespace "silo::schema":
    cdef enum class ColumnType(unsigned char):
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