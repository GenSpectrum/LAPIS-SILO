"""Tests for update_column."""

import datetime

import pytest

from .helpers import INPUT_FILE, SERIALIZED_STATE_DIR, create_nucleotide_sequence_table


class TestUpdateColumn:
    """Test the update_column binding.

    Extra string columns added alongside a sequence column are plain (non-indexed, non-phylo)
    string columns, so these tests cover both the binding layer itself (argument validation,
    marshalling, translation of C++ errors into Python exceptions) and the end-to-end update of a
    string column. Scalar value column updates are additionally exercised via create_table in
    TestCreateTable in test_create_table.py and against the loaded database in
    TestUpdateColumnOnLoadedDatabase below.
    """

    def _database_with_string_column(self, main_reference_sequence):
        from rhydb import Database

        db = Database()
        create_nucleotide_sequence_table(db,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence,
            extra_columns=["country"]
        )
        db.append_data_from_file("sequences", INPUT_FILE)
        return db

    @staticmethod
    def _count(database, filter_expression):
        return len(database.get_filtered_bitmap("sequences", filter_expression))

    def test_update_column_empty_table_name_raises(self, empty_database):
        """Test that an empty table name raises ValueError before reaching C++."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.update_column("", "country", "'x'")

    def test_update_column_empty_column_name_raises(self, empty_database):
        """Test that an empty column name raises ValueError before reaching C++."""
        with pytest.raises(ValueError, match="column_name cannot be empty"):
            empty_database.update_column("sequences", "", "'x'")

    def test_update_column_unknown_column_raises(self, main_reference_sequence):
        """Test that updating a non-existent column surfaces the C++ query error as ValueError."""
        db = self._database_with_string_column(main_reference_sequence)
        with pytest.raises(ValueError, match="does not contain a column"):
            db.update_column("sequences", "does_not_exist", "1")

    def test_update_string_column_all_rows(self, main_reference_sequence):
        """A quoted string literal is assigned to every matched row of a string column."""
        db = self._database_with_string_column(main_reference_sequence)
        total = self._count(db, "true")
        assert self._count(db, "country = 'Switzerland'") == total

        db.update_column("sequences", "country", "'Germany'")

        assert self._count(db, "country = 'Switzerland'") == 0
        assert self._count(db, "country = 'Germany'") == total

    def test_update_string_column_with_filter(self, main_reference_sequence):
        """Only rows matching the filter are reassigned; a brand-new value is interned."""
        db = self._database_with_string_column(main_reference_sequence)
        total = self._count(db, "true")
        assert total > 1

        db.update_column("sequences", "country", "'France'", "primary_key = 'key_29'")

        assert self._count(db, "country = 'France'") == 1
        assert self._count(db, "country = 'Switzerland'") == total - 1

    def test_update_string_column_clears_rows_to_null(self, main_reference_sequence):
        """The literal 'null' clears the matched rows of a string column."""
        db = self._database_with_string_column(main_reference_sequence)
        assert self._count(db, "country = null") == 0

        db.update_column("sequences", "country", "null", "primary_key = 'key_29'")

        assert self._count(db, "country = null") == 1

    def test_update_string_column_type_mismatch_raises(self, main_reference_sequence):
        """A non-string literal is rejected for a string column."""
        db = self._database_with_string_column(main_reference_sequence)
        with pytest.raises(ValueError, match="expected string literal"):
            db.update_column("sequences", "country", "5")


class TestUpdateColumnOnLoadedDatabase:
    """End-to-end update_column tests against the serialized database checked into the repo.

    That database has real scalar value columns (age:int, qc_value:float, date:date,
    test_boolean_column:bool) plus indexed string columns, and exercises the on-disk load path.
    update_column mutates only the in-memory database (the on-disk state is never saved), so every
    test loads its own fresh copy and the repo files are left untouched.
    """

    @pytest.fixture
    def loaded_database(self):
        from rhydb import Database
        return Database(SERIALIZED_STATE_DIR)

    @staticmethod
    def _column(database, name):
        return database.query(f'default.project({{{name}}})').to_pydict()[name]

    @staticmethod
    def _count(database, filter_expression):
        return len(database.get_filtered_bitmap("default", filter_expression))

    def test_update_int_column_all_rows(self, loaded_database):
        """Updating without a filter assigns the value to every row."""
        total = self._count(loaded_database, "true")
        assert total > 0

        loaded_database.update_column("default", "age", "42")

        assert self._count(loaded_database, "age = 42") == total
        assert all(age == 42 for age in self._column(loaded_database, "age"))

    def test_update_int_column_with_filter(self, loaded_database):
        """Only rows matching the filter are updated; the rest keep their values."""
        matching = self._count(loaded_database, "age = 4")
        total = self._count(loaded_database, "true")
        assert 0 < matching < total

        loaded_database.update_column("default", "age", "100", "age = 4")

        assert self._count(loaded_database, "age = 4") == 0
        assert self._count(loaded_database, "age = 100") == matching
        # Rows that did not match still exist and were not turned into 100.
        assert self._count(loaded_database, "age = 100") < total

    def test_update_float_column(self, loaded_database):
        """Float literals are assigned to a float column."""
        loaded_database.update_column("default", "qc_value", "0.5")
        assert all(abs(value - 0.5) < 1e-9 for value in self._column(loaded_database, "qc_value"))

    def test_update_bool_column(self, loaded_database):
        """Boolean literals are assigned to a bool column (including rows that were null)."""
        loaded_database.update_column("default", "test_boolean_column", "false")
        assert all(value is False for value in self._column(loaded_database, "test_boolean_column"))

    def test_update_date_column(self, loaded_database):
        """SaneQL date literals are assigned to a date column."""
        loaded_database.update_column("default", "date", "'2000-01-01'::date")
        assert all(
            value == datetime.date(2000, 1, 1) for value in self._column(loaded_database, "date")
        )

    def test_update_column_clears_rows_to_null(self, loaded_database):
        """The literal 'null' clears the matched rows."""
        matching = self._count(loaded_database, "age = 4")
        nulls_before = self._count(loaded_database, "age = null")
        assert matching > 0

        loaded_database.update_column("default", "age", "null", "age = 4")

        assert self._count(loaded_database, "age = 4") == 0
        assert self._count(loaded_database, "age = null") == nulls_before + matching

    def test_update_does_not_persist_to_disk(self, loaded_database):
        """Updating the in-memory database must not modify the on-disk repo state."""
        loaded_database.update_column("default", "age", "999")
        assert self._count(loaded_database, "age = 999") > 0

        from rhydb import Database
        fresh = Database(SERIALIZED_STATE_DIR)
        assert self._count(fresh, "age = 999") == 0
