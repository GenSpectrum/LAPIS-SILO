"""Tests for get_tables."""

import pyarrow as pa

from .helpers import create_gene_table, create_nucleotide_sequence_table


class TestGetTables:
    """Test the get_tables method."""

    def test_get_tables_empty_database(self, empty_database):
        """Even an empty database lists the built-in, always-present bookkeeping tables."""
        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        assert "table_name" in result.column_names
        assert set(result.to_pydict()["table_name"]) == {"reference_genomes", "reference_columns"}

    def test_get_tables_single_table(self, empty_database, main_reference_sequence):
        """Test get_tables with one table."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        assert "table_name" in result.column_names
        # The built-in bookkeeping tables are normal, visible tables.
        table_names = set(result.to_pydict()["table_name"])
        assert table_names == {"sequences", "reference_genomes", "reference_columns"}

    def test_get_tables_multiple_tables(self, empty_database, main_reference_sequence):
        """Test get_tables with multiple tables."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        create_gene_table(empty_database,
            table_name="genes",
            primary_key_name="id",
            gene_name="S",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*"
        )

        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        # The built-in bookkeeping tables are normal, visible tables.
        table_names = set(result.to_pydict()["table_name"])
        assert table_names == {"sequences", "genes", "reference_genomes", "reference_columns"}
