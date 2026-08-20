"""Tests for get_filtered_bitmap."""

import pyroaring

from .helpers import INPUT_FILE, create_nucleotide_sequence_table


class TestGetFilteredBitmap:
    """Test getting filtered bitmaps."""

    def test_get_filtered_bitmap_true_filter(self, empty_database, main_reference_sequence):
        """Test getting a bitmap with True filter (returns all rows)."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # True filter should return all rows
        bitmap = empty_database.get_filtered_bitmap("sequences", 'true')
        assert isinstance(bitmap, pyroaring.BitMap)
        assert len(bitmap) > 0  # Should have at least one row from test data

    def test_get_filtered_bitmap_returns_bitmap(self, empty_database, main_reference_sequence):
        """Test that get_filtered_bitmap returns a pyroaring.BitMap."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", 'true')
        assert isinstance(bitmap, pyroaring.BitMap)
        # Can iterate over bitmap to get indices
        indices = list(bitmap)
        for idx in indices:
            assert isinstance(idx, int)

    def test_get_filtered_bitmap_with_none_filter(self, empty_database, main_reference_sequence):
        """Test that None filter defaults to True filter."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", None)
        assert isinstance(bitmap, pyroaring.BitMap)

    def test_get_filtered_bitmap_with_empty_filter(self, empty_database, main_reference_sequence):
        """Test that empty string filter defaults to True filter."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", "")
        assert isinstance(bitmap, pyroaring.BitMap)
        assert len(bitmap) > 0

    def test_get_filtered_bitmap_supports_set_operations(self, empty_database, main_reference_sequence):
        """Test that returned bitmap supports set operations."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", 'true')

        # Test union, intersection operations
        other_bitmap = pyroaring.BitMap([0, 1, 2])
        union = bitmap | other_bitmap
        intersection = bitmap & other_bitmap

        assert isinstance(union, pyroaring.BitMap)
        assert isinstance(intersection, pyroaring.BitMap)
