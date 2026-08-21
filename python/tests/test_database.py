"""Tests for module import, database construction and loading."""

import pytest


class TestDatabaseImport:
    """Test that the rhydb module can be imported correctly."""

    def test_import_database(self):
        """Test that Database can be imported from rhydb."""
        from rhydb import Database
        assert Database is not None

    def test_import_pydatabase(self):
        """Test that PyDatabase can be imported from rhydb.database."""
        from rhydb.database import PyDatabase
        assert PyDatabase is not None


class TestDatabaseCreation:
    """Test database creation and basic operations."""

    def test_create_empty_database(self):
        """Test creating an empty database."""
        from rhydb import Database
        db = Database()
        assert db is not None

    def test_database_has_expected_methods(self, empty_database):
        """Test that the database has all expected methods."""
        expected_methods = [
            'append_data_from_file',
            'append_data_from_string',
            'create_table',
            'register_reference',
            'query',
            'get_filtered_bitmap',
            'get_nucleotide_reference_sequence',
            'get_amino_acid_reference_sequence',
            'get_tables',
            'print_all_data',
            'save_checkpoint',
            'update_column',
        ]
        for method in expected_methods:
            assert hasattr(empty_database, method), f"Missing method: {method}"

    def test_database_repr(self, empty_database):
        """Test database string representation."""
        assert repr(empty_database) == "Database()"

    def test_database_context_manager(self):
        """Test that database can be used as a context manager."""
        from rhydb import Database
        with Database() as db:
            assert db is not None


class TestDatabaseLoad:
    """Test database loading from saved state."""

    def test_load_nonexistent_path(self):
        """Test that loading from nonexistent path raises FileNotFoundError."""
        from rhydb import Database
        with pytest.raises(FileNotFoundError):
            Database("/nonexistent/path/to/database")
