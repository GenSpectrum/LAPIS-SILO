"""Tests for argument validation of the Database methods."""

import pytest


class TestDatabaseValidation:
    """Test input validation for database methods."""

    def test_append_data_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.append_data_from_file("", "/some/file.ndjson")

    def test_append_data_empty_file_name(self, empty_database):
        """Test that empty file name raises ValueError."""
        with pytest.raises(ValueError, match="file_name cannot be empty"):
            empty_database.append_data_from_file("test_table", "")

    def test_append_data_nonexistent_file(self, empty_database):
        """Test that nonexistent file raises FileNotFoundError."""
        with pytest.raises(FileNotFoundError):
            empty_database.append_data_from_file("test_table", "/nonexistent/file.ndjson")

    def test_print_all_data_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.print_all_data("")

    def test_get_nucleotide_reference_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.get_nucleotide_reference_sequence("", "main")

    def test_get_nucleotide_reference_empty_sequence_name(self, empty_database):
        """Test that empty sequence name raises ValueError."""
        with pytest.raises(ValueError, match="sequence_name cannot be empty"):
            empty_database.get_nucleotide_reference_sequence("test", "")

    def test_get_filtered_bitmap_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.get_filtered_bitmap("", "filter")

    def test_save_checkpoint_empty_directory(self, empty_database):
        """Test that empty directory raises ValueError."""
        with pytest.raises(ValueError, match="save_directory cannot be empty"):
            empty_database.save_checkpoint("")
