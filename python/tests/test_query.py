"""Tests for the query method returning PyArrow Tables."""

import os

import pyarrow as pa
import pytest

from .helpers import INPUT_FILE, create_nucleotide_sequence_table


class TestQuery:
    """Test the query method that returns PyArrow Tables."""

    def test_query_returns_pyarrow_table(self, empty_database, main_reference_sequence):
        """Test that query returns a PyArrow Table."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = 'sequences'

        result = empty_database.query(query)

        assert isinstance(result, pa.Table)

    def test_query_has_correct_schema(self, empty_database, main_reference_sequence):
        """Test that the returned table has expected columns."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = 'sequences'
        result = empty_database.query(query)

        # Should have at least the primary key column
        assert "primary_key" in result.column_names

    def test_query_returns_data(self, empty_database, main_reference_sequence):
        """Test that query returns rows."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = 'sequences'
        result = empty_database.query(query)

        assert result.num_rows > 0

    def test_query_with_filter(self, empty_database, main_reference_sequence):
        """Test query with a filter expression."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Get all rows first
        all_query = 'sequences'
        all_result = empty_database.query(all_query)

        # Get filtered rows (False filter should return 0 rows)
        filtered_query = 'sequences.filter(false)'
        filtered_result = empty_database.query(filtered_query)

        assert filtered_result.num_rows == 0
        assert all_result.num_rows > filtered_result.num_rows

    def test_query_to_batches(self, empty_database, main_reference_sequence):
        """Test that the result can be converted to RecordBatches."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = 'sequences'
        result = empty_database.query(query)

        batches = result.to_batches()
        assert isinstance(batches, list)
        for batch in batches:
            assert isinstance(batch, pa.RecordBatch)

    def test_query_to_pydict(self, empty_database, main_reference_sequence):
        """Test that the result can be converted to Python dict."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = 'sequences'
        result = empty_database.query(query)

        data = result.to_pydict()
        assert isinstance(data, dict)
        assert "primary_key" in data
        assert isinstance(data["primary_key"], list)

    def test_query_empty_query_raises(self, empty_database, main_reference_sequence):
        """Test that empty query raises ValueError."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        with pytest.raises(ValueError, match="query_string cannot be empty"):
            empty_database.query("")

    def test_query_invalid_query_raises(self, empty_database, main_reference_sequence):
        """Test that invalid SaneQL raises an error."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        with pytest.raises(RuntimeError):
            empty_database.query("not valid saneql !")

    def test_query_simple_database(self):
        """Test query with a simple in-memory database."""
        from rhydb import Database

        db = Database()
        create_nucleotide_sequence_table(db,
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT"
        )
        db.append_data_from_string("test", '{"id": "sample1", "seq": {"sequence": "AAAA", "insertions": []}}')
        db.append_data_from_string("test", '{"id": "sample2", "seq": {"sequence": "CCCC", "insertions": []}}')

        query = 'test'
        result = db.query(query)

        assert isinstance(result, pa.Table)
        assert result.num_rows == 2
        assert "id" in result.column_names

        # Verify the data
        data = result.to_pydict()
        assert set(data["id"]) == {"sample1", "sample2"}

    def test_query_preserves_data_after_checkpoint(self, empty_database, main_reference_sequence, temp_dir):
        """Test that query works correctly after loading from checkpoint."""
        from rhydb import Database

        # Create and populate database
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Query before checkpoint
        query = 'sequences'
        result_before = empty_database.query(query)

        # Save and reload
        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)
        loaded_db = Database(save_path)

        # Query after checkpoint
        result_after = loaded_db.query(query)

        # Results should match
        assert result_before.num_rows == result_after.num_rows
        assert result_before.column_names == result_after.column_names
