import pytest
import os
import json
import tempfile
import shutil
import pyroaring
import pyarrow as pa


# Path to test data
TEST_DATA_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'testBaseData', 'exampleDataset')
REFERENCE_GENOMES_FILE = os.path.join(TEST_DATA_DIR, 'reference_genomes.json')
INPUT_FILE = os.path.join(TEST_DATA_DIR, 'input_file.ndjson')


@pytest.fixture
def reference_genomes():
    """Load reference genomes from test data."""
    with open(REFERENCE_GENOMES_FILE, 'r') as f:
        return json.load(f)


@pytest.fixture
def main_reference_sequence(reference_genomes):
    """Get the main nucleotide reference sequence."""
    for seq in reference_genomes['nucleotideSequences']:
        if seq['name'] == 'main':
            return seq['sequence']
    raise ValueError("No 'main' sequence found in reference genomes")


class TestDatabaseImport:
    """Test that the pysilo module can be imported correctly."""

    def test_import_database(self):
        """Test that Database can be imported from pysilo."""
        from pysilo import Database
        assert Database is not None

    def test_import_pydatabase(self):
        """Test that PyDatabase can be imported from pysilo.database."""
        from pysilo.database import PyDatabase
        assert PyDatabase is not None


class TestDatabaseCreation:
    """Test database creation and basic operations."""

    def test_create_empty_database(self):
        """Test creating an empty database."""
        from pysilo import Database
        db = Database()
        assert db is not None

    def test_database_has_expected_methods(self, empty_database):
        """Test that the database has all expected methods."""
        expected_methods = [
            'append_data_from_file',
            'append_data_from_string',
            'create_gene_table',
            'create_nucleotide_sequence_table',
            'execute_query',
            'get_filtered_bitmap',
            'get_nucleotide_reference_sequence',
            'get_amino_acid_reference_sequence',
            'get_prevalent_nucleotide_mutations',
            'get_prevalent_amino_acid_mutations',
            'get_tables',
            'print_all_data',
            'save_checkpoint',
        ]
        for method in expected_methods:
            assert hasattr(empty_database, method), f"Missing method: {method}"

    def test_database_repr(self, empty_database):
        """Test database string representation."""
        assert repr(empty_database) == "Database()"

    def test_database_context_manager(self):
        """Test that database can be used as a context manager."""
        from pysilo import Database
        with Database() as db:
            assert db is not None


class TestCreateNucleotideSequenceTable:
    """Test creating nucleotide sequence tables."""

    def test_create_table_with_simple_reference(self, empty_database):
        """Test creating a table with a simple reference sequence."""
        # Table names must be lowercase
        empty_database.create_nucleotide_sequence_table(
            table_name="testtable",
            primary_key_name="id",
            sequence_name="main",
            reference_sequence="ACGTACGTACGT"
        )
        # If no exception, table was created

    def test_create_table_with_real_reference(self, empty_database, main_reference_sequence):
        """Test creating a table with the real SARS-CoV-2 reference sequence."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="gisaidepisl",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        # If no exception, table was created

    def test_get_reference_sequence_after_create(self, empty_database, main_reference_sequence):
        """Test that we can retrieve the reference sequence after creating a table."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        retrieved = empty_database.get_nucleotide_reference_sequence("sequences", "main")
        assert retrieved == main_reference_sequence

    def test_get_gene_reference_sequence_after_create(self, empty_database):
        """Test that we can retrieve the reference sequence after creating a table."""
        empty_database.create_gene_table(
            table_name="sequences",
            primary_key_name="key",
            gene_name="main",
            reference_sequence="ABCD"
        )

        retrieved = empty_database.get_amino_acid_reference_sequence("sequences", "main")
        assert retrieved == "ABCD"


class TestCreateGeneTable:
    """Test creating gene tables."""

    def test_create_gene_table(self, empty_database):
        """Test creating a gene table."""
        # Table names must be lowercase
        empty_database.create_gene_table(
            table_name="genes",
            primary_key_name="id",
            gene_name="S",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*"
        )
        # If no exception, table was created


class TestAppendData:
    """Test appending data to tables."""

    def test_append_data_from_real_file(self, empty_database, main_reference_sequence):
        """Test appending data from the real test data file."""
        # First create the table
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        # Then append data
        empty_database.append_data_from_file("sequences", INPUT_FILE)
        # If no exception, data was appended


class TestGetFilteredBitmap:
    """Test getting filtered bitmaps."""

    def test_get_filtered_bitmap_true_filter(self, empty_database, main_reference_sequence):
        """Test getting a bitmap with True filter (returns all rows)."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # True filter should return all rows
        bitmap = empty_database.get_filtered_bitmap("sequences", '{"type":"True"}')
        assert isinstance(bitmap, pyroaring.BitMap)
        assert len(bitmap) > 0  # Should have at least one row from test data

    def test_get_filtered_bitmap_returns_bitmap(self, empty_database, main_reference_sequence):
        """Test that get_filtered_bitmap returns a pyroaring.BitMap."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", '{"type":"True"}')
        assert isinstance(bitmap, pyroaring.BitMap)
        # Can iterate over bitmap to get indices
        indices = list(bitmap)
        for idx in indices:
            assert isinstance(idx, int)

    def test_get_filtered_bitmap_with_none_filter(self, empty_database, main_reference_sequence):
        """Test that None filter defaults to True filter."""
        empty_database.create_nucleotide_sequence_table(
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
        empty_database.create_nucleotide_sequence_table(
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
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        bitmap = empty_database.get_filtered_bitmap("sequences", '{"type":"True"}')

        # Test union, intersection operations
        other_bitmap = pyroaring.BitMap([0, 1, 2])
        union = bitmap | other_bitmap
        intersection = bitmap & other_bitmap

        assert isinstance(union, pyroaring.BitMap)
        assert isinstance(intersection, pyroaring.BitMap)


class TestGetPrevalentMutations:
    """Test getting prevalent mutations."""

    def test_get_prevalent_mutations_basic(self, empty_database, main_reference_sequence):
        """Test getting prevalent mutations."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        mutations = empty_database.get_prevalent_nucleotide_mutations(
            table_name="sequences",
            sequence_name="main",
            prevalence_threshold=0.5,
            filter_expression='{"type":"True"}'
        )
        assert isinstance(mutations, list)

    def test_get_prevalent_mutations_returns_tuples(self, empty_database, main_reference_sequence):
        """Test that mutations are returned as (position, mutation) tuples."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        mutations = empty_database.get_prevalent_nucleotide_mutations(
            table_name="sequences",
            sequence_name="main",
            prevalence_threshold=0.0,  # Get all mutations
            filter_expression='{"type":"True"}'
        )

        for mutation in mutations:
            assert isinstance(mutation, tuple)
            assert len(mutation) == 2
            assert isinstance(mutation[0], int)  # position
            assert isinstance(mutation[1], str)  # mutation string

    def test_get_prevalent_mutations_threshold_filtering(self, empty_database, main_reference_sequence):
        """Test that higher thresholds return fewer mutations."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        low_threshold = empty_database.get_prevalent_nucleotide_mutations(
            "sequences", "main", 0.1, '{"type":"True"}'
        )
        high_threshold = empty_database.get_prevalent_nucleotide_mutations(
            "sequences", "main", 0.9, '{"type":"True"}'
        )

        # Higher threshold should return same or fewer mutations
        assert len(high_threshold) <= len(low_threshold)


class TestSaveAndLoadCheckpoint:
    """Test saving and loading database checkpoints."""

    def test_save_checkpoint(self, empty_database, main_reference_sequence, temp_dir):
        """Test saving a database checkpoint."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)

        # Check that something was saved
        assert os.path.exists(save_path)

    def test_load_from_checkpoint(self, empty_database, main_reference_sequence, temp_dir):
        """Test loading a database from a checkpoint."""
        from pysilo import Database

        # Create and save
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)

        # Load from checkpoint
        loaded_db = Database(save_path)
        assert loaded_db is not None

        # Verify data is preserved
        ref_seq = loaded_db.get_nucleotide_reference_sequence("sequences", "main")
        assert ref_seq == main_reference_sequence

    def test_checkpoint_preserves_data(self, empty_database, main_reference_sequence, temp_dir):
        """Test that checkpoint preserves all data correctly."""
        from pysilo import Database

        # Create, add data, and save
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Get data before save
        bitmap_before = empty_database.get_filtered_bitmap("sequences", '{"type":"True"}')
        mutations_before = empty_database.get_prevalent_nucleotide_mutations(
            "sequences", "main", 0.5, '{"type":"True"}'
        )

        # Save and reload
        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)

        loaded_db = Database(save_path)

        # Compare with loaded data
        bitmap_after = loaded_db.get_filtered_bitmap("sequences", '{"type":"True"}')
        mutations_after = loaded_db.get_prevalent_nucleotide_mutations(
            "sequences", "main", 0.5, '{"type":"True"}'
        )

        assert bitmap_before == bitmap_after
        assert mutations_before == mutations_after


class TestDatabaseValidation:
    """Test input validation for database methods."""

    def test_create_nucleotide_table_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.create_nucleotide_sequence_table(
                table_name="",
                primary_key_name="id",
                sequence_name="main",
                reference_sequence="ACGT"
            )

    def test_create_nucleotide_table_empty_primary_key(self, empty_database):
        """Test that empty primary key name raises ValueError."""
        with pytest.raises(ValueError, match="primary_key_name cannot be empty"):
            empty_database.create_nucleotide_sequence_table(
                table_name="test",
                primary_key_name="",
                sequence_name="main",
                reference_sequence="ACGT"
            )

    def test_create_nucleotide_table_empty_sequence_name(self, empty_database):
        """Test that empty sequence name raises ValueError."""
        with pytest.raises(ValueError, match="sequence_name cannot be empty"):
            empty_database.create_nucleotide_sequence_table(
                table_name="test",
                primary_key_name="id",
                sequence_name="",
                reference_sequence="ACGT"
            )

    def test_create_nucleotide_table_empty_reference(self, empty_database):
        """Test that empty reference sequence raises ValueError."""
        with pytest.raises(ValueError, match="reference_sequence cannot be empty"):
            empty_database.create_nucleotide_sequence_table(
                table_name="test",
                primary_key_name="id",
                sequence_name="main",
                reference_sequence=""
            )

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

    def test_get_prevalent_mutations_empty_table_name(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.get_prevalent_nucleotide_mutations("", "main", 0.5)

    def test_get_prevalent_mutations_empty_sequence_name(self, empty_database):
        """Test that empty sequence name raises ValueError."""
        with pytest.raises(ValueError, match="sequence_name cannot be empty"):
            empty_database.get_prevalent_nucleotide_mutations("test", "", 0.5)

    def test_get_prevalent_mutations_invalid_threshold_low(self, empty_database):
        """Test that threshold below 0 raises ValueError."""
        with pytest.raises(ValueError, match="prevalence_threshold must be between"):
            empty_database.get_prevalent_nucleotide_mutations("test", "main", -0.1)

    def test_get_prevalent_mutations_invalid_threshold_high(self, empty_database):
        """Test that threshold above 1 raises ValueError."""
        with pytest.raises(ValueError, match="prevalence_threshold must be between"):
            empty_database.get_prevalent_nucleotide_mutations("test", "main", 1.5)

    def test_save_checkpoint_empty_directory(self, empty_database):
        """Test that empty directory raises ValueError."""
        with pytest.raises(ValueError, match="save_directory cannot be empty"):
            empty_database.save_checkpoint("")


class TestDatabaseLoad:
    """Test database loading from saved state."""

    def test_load_nonexistent_path(self):
        """Test that loading from nonexistent path raises FileNotFoundError."""
        from pysilo import Database
        with pytest.raises(FileNotFoundError):
            Database("/nonexistent/path/to/database")


class TestPrintAllData:
    """Test the print_all_data method."""

    def test_print_all_data(self, empty_database, main_reference_sequence, capsys):
        """Test that print_all_data outputs something."""
        # Note: printAllData expects sequence_name="sequence" (hardcoded in C++)
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # This should print to stdout
        empty_database.print_all_data("sequences")

        # Check that something was printed
        captured = capsys.readouterr()
        # The output goes to C++ stdout, so we may not capture it in Python
        # Just verify no exception was raised


class TestExtraColumns:
    """Test creating tables with extra string columns."""

    def test_create_table_with_extra_columns(self):
        """Test creating a table with extra string columns."""
        from pysilo import Database

        db = Database()
        db.create_nucleotide_sequence_table(
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=["country", "date", "lineage"]
        )
        # If no exception, table was created with extra columns

    def test_extra_columns_accept_data(self):
        """Test that extra columns can store and retrieve data."""
        from pysilo import Database

        db = Database()
        db.create_nucleotide_sequence_table(
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=["country", "lineage"]
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}, "country": "USA", "lineage": "BA.1"}')
        db.append_data_from_string("test", '{"id": "s2", "seq": {"sequence": "CCCC", "insertions": []}, "country": "UK", "lineage": "BA.2"}')

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = db.execute_query("test", query)

        assert "country" in result.column_names
        assert "lineage" in result.column_names
        data = result.to_pydict()
        assert set(data["country"]) == {"USA", "UK"}
        assert set(data["lineage"]) == {"BA.1", "BA.2"}

    def test_extra_columns_default_empty(self):
        """Test that extra_columns defaults to empty (backward compatible)."""
        from pysilo import Database

        db = Database()
        # Call without extra_columns - should work as before
        db.create_nucleotide_sequence_table(
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT"
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}}')

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = db.execute_query("test", query)
        assert result.num_rows == 1

    def test_extra_columns_with_none(self):
        """Test that extra_columns=None works."""
        from pysilo import Database

        db = Database()
        db.create_nucleotide_sequence_table(
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=None
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}}')

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = db.execute_query("test", query)
        assert result.num_rows == 1

    def test_extra_columns_invalid_type_raises(self):
        """Test that non-string extra columns raise TypeError."""
        from pysilo import Database

        db = Database()
        with pytest.raises(TypeError, match="extra_columns must contain strings"):
            db.create_nucleotide_sequence_table(
                table_name="test",
                primary_key_name="id",
                sequence_name="seq",
                reference_sequence="ACGT",
                extra_columns=["valid", 123]  # 123 is not a string
            )

    def test_gene_table_with_extra_columns(self):
        """Test creating a gene table with extra columns."""
        from pysilo import Database

        db = Database()
        db.create_gene_table(
            table_name="genes",
            primary_key_name="id",
            gene_name="spike",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*",
            extra_columns=["variant", "source"]
        )
        # If no exception, table was created


class TestGetTables:
    """Test the get_tables method."""

    def test_get_tables_empty_database(self, empty_database):
        """Test that get_tables returns empty table for empty database."""
        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        assert "table_name" in result.column_names
        assert result.num_rows == 0

    def test_get_tables_single_table(self, empty_database, main_reference_sequence):
        """Test get_tables with one table."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        assert "table_name" in result.column_names
        assert result.num_rows == 1
        data = result.to_pydict()
        assert "sequences" in data["table_name"]

    def test_get_tables_multiple_tables(self, empty_database, main_reference_sequence):
        """Test get_tables with multiple tables."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.create_gene_table(
            table_name="genes",
            primary_key_name="id",
            gene_name="S",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*"
        )

        result = empty_database.get_tables()
        assert isinstance(result, pa.Table)
        assert result.num_rows == 2
        data = result.to_pydict()
        table_names = set(data["table_name"])
        assert "sequences" in table_names
        assert "genes" in table_names


class TestExecuteQuery:
    """Test the execute_query method that returns PyArrow Tables."""

    def test_execute_query_returns_pyarrow_table(self, empty_database, main_reference_sequence):
        """Test that execute_query returns a PyArrow Table."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = empty_database.execute_query("sequences", query)

        assert isinstance(result, pa.Table)

    def test_execute_query_has_correct_schema(self, empty_database, main_reference_sequence):
        """Test that the returned table has expected columns."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = empty_database.execute_query("sequences", query)

        # Should have at least the primary key column
        assert "primary_key" in result.column_names

    def test_execute_query_returns_data(self, empty_database, main_reference_sequence):
        """Test that execute_query returns rows."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = empty_database.execute_query("sequences", query)

        assert result.num_rows > 0

    def test_execute_query_with_filter(self, empty_database, main_reference_sequence):
        """Test execute_query with a filter expression."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Get all rows first
        all_query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        all_result = empty_database.execute_query("sequences", all_query)

        # Get filtered rows (False filter should return 0 rows)
        filtered_query = '{"filterExpression": {"type": "False"}, "action": {"type": "Details"}}'
        filtered_result = empty_database.execute_query("sequences", filtered_query)

        assert filtered_result.num_rows == 0
        assert all_result.num_rows > filtered_result.num_rows

    def test_execute_query_to_batches(self, empty_database, main_reference_sequence):
        """Test that the result can be converted to RecordBatches."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = empty_database.execute_query("sequences", query)

        batches = result.to_batches()
        assert isinstance(batches, list)
        for batch in batches:
            assert isinstance(batch, pa.RecordBatch)

    def test_execute_query_to_pydict(self, empty_database, main_reference_sequence):
        """Test that the result can be converted to Python dict."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = empty_database.execute_query("sequences", query)

        data = result.to_pydict()
        assert isinstance(data, dict)
        assert "primary_key" in data
        assert isinstance(data["primary_key"], list)

    def test_execute_query_empty_table_name_raises(self, empty_database):
        """Test that empty table name raises ValueError."""
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.execute_query("", '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}')

    def test_execute_query_empty_query_raises(self, empty_database, main_reference_sequence):
        """Test that empty query raises ValueError."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        with pytest.raises(ValueError, match="query_json cannot be empty"):
            empty_database.execute_query("sequences", "")

    def test_execute_query_invalid_json_raises(self, empty_database, main_reference_sequence):
        """Test that invalid JSON raises an error."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        with pytest.raises(ValueError, match="not a valid JSON"):
            empty_database.execute_query("sequences", "not valid json")

    def test_execute_query_missing_action_raises(self, empty_database, main_reference_sequence):
        """Test that query without action raises an error."""
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        with pytest.raises(ValueError, match="must contain filterExpression and action"):
            empty_database.execute_query("sequences", '{"filterExpression": {"type": "True"}}')

    def test_execute_query_simple_database(self):
        """Test execute_query with a simple in-memory database."""
        from pysilo import Database

        db = Database()
        db.create_nucleotide_sequence_table(
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT"
        )
        db.append_data_from_string("test", '{"id": "sample1", "seq": {"sequence": "AAAA", "insertions": []}}')
        db.append_data_from_string("test", '{"id": "sample2", "seq": {"sequence": "CCCC", "insertions": []}}')

        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result = db.execute_query("test", query)

        assert isinstance(result, pa.Table)
        assert result.num_rows == 2
        assert "id" in result.column_names

        # Verify the data
        data = result.to_pydict()
        assert set(data["id"]) == {"sample1", "sample2"}

    def test_execute_query_preserves_data_after_checkpoint(self, empty_database, main_reference_sequence, temp_dir):
        """Test that execute_query works correctly after loading from checkpoint."""
        from pysilo import Database

        # Create and populate database
        empty_database.create_nucleotide_sequence_table(
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Query before checkpoint
        query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        result_before = empty_database.execute_query("sequences", query)

        # Save and reload
        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)
        loaded_db = Database(save_path)

        # Query after checkpoint
        result_after = loaded_db.execute_query("sequences", query)

        # Results should match
        assert result_before.num_rows == result_after.num_rows
        assert result_before.column_names == result_after.column_names
