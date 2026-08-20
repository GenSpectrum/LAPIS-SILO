"""Tests for create_table and the sequence/gene table helpers built on top of it."""

import json

import pytest

from .helpers import create_gene_table, create_nucleotide_sequence_table


class TestCreateTable:
    """Test the generic create_table method that accepts columns of any supported type."""

    @staticmethod
    def _populate_references(db, entries):
        """Populate the built-in `reference_genomes` table create_table reads references from.

        `entries` maps a sequence column name to its reference string.
        """
        for name, reference in entries.items():
            db.register_reference(name, reference)

    def test_create_table_with_all_column_types(self):
        """A table can be created with every supported column type and populated."""
        from rhydb import Database

        db = Database()
        self._populate_references(db, {"seq": "ACGT", "gene": "MFV"})
        db.create_table(
            table_name="samples",
            columns=[
                {"name": "id", "type": "string"},  # first column -> primary key
                {"name": "age", "type": "int"},
                {"name": "qc", "type": "float"},
                {"name": "collected", "type": "date"},
                {"name": "passed", "type": "bool"},
                {"name": "country", "type": "string"},
                {"name": "lineage", "type": "indexed_string"},
                {"name": "seq", "type": "nucleotide_sequence"},
                {"name": "gene", "type": "amino_acid_sequence"},
            ],
        )
        db.append_data_from_string(
            "samples",
            json.dumps(
                {
                    "id": "s1",
                    "age": 42,
                    "qc": 0.5,
                    "collected": "2021-03-15",
                    "passed": True,
                    "country": "Switzerland",
                    "lineage": "B.1",
                    "seq": {"sequence": "ACGT", "insertions": []},
                    "gene": {"sequence": "MFV", "insertions": []},
                }
            ),
        )

        result = db.query("samples.project({id, age, qc, country, lineage})")
        data = result.to_pydict()
        assert data["id"] == ["s1"]
        assert data["age"] == [42]
        assert data["qc"] == [0.5]
        assert data["country"] == ["Switzerland"]
        assert data["lineage"] == ["B.1"]
        # The sequence column's reference was resolved from the `reference_genomes` table by column name.
        assert db.get_nucleotide_reference_sequence("samples", "seq") == "ACGT"
        assert db.get_amino_acid_reference_sequence("samples", "gene") == "MFV"

    def test_create_table_reference_looked_up_by_column_name(self):
        """A sequence column's reference is taken from the matching `reference_genomes` entry."""
        from rhydb import Database

        db = Database()
        self._populate_references(db, {"main": "ACGTACGT"})
        db.create_table(
            "sequences",
            [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
        )

        assert db.get_nucleotide_reference_sequence("sequences", "main") == "ACGTACGT"

    def test_create_table_missing_reference_entry_raises(self):
        """Creating a sequence column with no matching `reference_genomes` entry fails."""
        from rhydb import Database

        db = Database()
        self._populate_references(db, {"other": "ACGT"})
        with pytest.raises(RuntimeError, match="no entry named 'seq'"):
            db.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "seq", "type": "nucleotide_sequence"}],
            )

    def test_create_table_scalar_columns_are_updatable(self):
        """Scalar columns created via create_table support update_column end to end."""
        from rhydb import Database

        db = Database()
        db.create_table(
            table_name="samples",
            columns=[{"name": "id", "type": "string"}, {"name": "age", "type": "int"}],
        )
        db.append_data_from_string("samples", '{"id": "s1", "age": 1}')
        db.append_data_from_string("samples", '{"id": "s2", "age": 2}')

        db.update_column("samples", "age", "99", "age = 1")

        assert len(db.get_filtered_bitmap("samples", "age = 99")) == 1
        assert len(db.get_filtered_bitmap("samples", "age = 2")) == 1

    def test_create_table_single_column_is_primary_key(self):
        """A table can be created with a single column, which becomes the primary key."""
        from rhydb import Database

        db = Database()
        db.create_table(table_name="samples", columns=[{"name": "id", "type": "string"}])
        db.append_data_from_string("samples", '{"id": "s1"}')

        result = db.query("samples")
        assert result.num_rows == 1
        assert "id" in result.column_names

    def test_create_table_empty_table_name_raises(self, empty_database):
        with pytest.raises(ValueError, match="table_name cannot be empty"):
            empty_database.create_table("", [{"name": "id", "type": "string"}])

    def test_create_table_no_columns_raises(self, empty_database):
        with pytest.raises(ValueError, match="at least one column"):
            empty_database.create_table("samples", [])

    def test_create_table_non_string_primary_key_raises(self, empty_database):
        with pytest.raises(RuntimeError, match="must be of type 'string'"):
            empty_database.create_table("samples", [{"name": "id", "type": "int"}])

    def test_create_table_unknown_type_raises(self, empty_database):
        with pytest.raises(ValueError, match="unknown type"):
            empty_database.create_table("samples", [{"name": "x", "type": "nonsense"}])

    def test_create_table_missing_name_raises(self, empty_database):
        with pytest.raises(ValueError, match="non-empty string 'name'"):
            empty_database.create_table("samples", [{"type": "int"}])

    def test_create_table_column_not_dict_raises(self, empty_database):
        with pytest.raises(TypeError, match="each column must be a dict"):
            empty_database.create_table("samples", ["age"])

    def test_create_table_duplicate_column_raises(self, empty_database):
        with pytest.raises(RuntimeError, match="Duplicate column name"):
            empty_database.create_table(
                "samples", [{"name": "x", "type": "string"}, {"name": "x", "type": "int"}]
            )


class TestCreateNucleotideSequenceTable:
    """Test creating nucleotide sequence tables."""

    def test_create_table_with_simple_reference(self, empty_database):
        """Test creating a table with a simple reference sequence."""
        # Table names must be lowercase
        create_nucleotide_sequence_table(empty_database,
            table_name="testtable",
            primary_key_name="id",
            sequence_name="main",
            reference_sequence="ACGTACGTACGT"
        )
        # If no exception, table was created

    def test_create_table_with_real_reference(self, empty_database, main_reference_sequence):
        """Test creating a table with the real SARS-CoV-2 reference sequence."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="gisaidepisl",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        # If no exception, table was created

    def test_get_reference_sequence_after_create(self, empty_database, main_reference_sequence):
        """Test that we can retrieve the reference sequence after creating a table."""
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        retrieved = empty_database.get_nucleotide_reference_sequence("sequences", "main")
        assert retrieved == main_reference_sequence

    def test_get_gene_reference_sequence_after_create(self, empty_database):
        """Test that we can retrieve the reference sequence after creating a table."""
        create_gene_table(empty_database,
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
        create_gene_table(empty_database,
            table_name="genes",
            primary_key_name="id",
            gene_name="S",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*"
        )
        # If no exception, table was created


class TestExtraColumns:
    """Test creating tables with extra string columns."""

    def test_create_table_with_extra_columns(self):
        """Test creating a table with extra string columns."""
        from rhydb import Database

        db = Database()
        create_nucleotide_sequence_table(db,
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=["country", "date", "lineage"]
        )
        # If no exception, table was created with extra columns

    def test_extra_columns_accept_data(self):
        """Test that extra columns can store and retrieve data."""
        from rhydb import Database

        db = Database()
        create_nucleotide_sequence_table(db,
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=["country", "lineage"]
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}, "country": "USA", "lineage": "BA.1"}')
        db.append_data_from_string("test", '{"id": "s2", "seq": {"sequence": "CCCC", "insertions": []}, "country": "UK", "lineage": "BA.2"}')

        query = 'test'
        result = db.query(query)

        assert "country" in result.column_names
        assert "lineage" in result.column_names
        data = result.to_pydict()
        assert set(data["country"]) == {"USA", "UK"}
        assert set(data["lineage"]) == {"BA.1", "BA.2"}

    def test_extra_columns_default_empty(self):
        """Test that extra_columns defaults to empty (backward compatible)."""
        from rhydb import Database

        db = Database()
        # Call without extra_columns - should work as before
        create_nucleotide_sequence_table(db,
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT"
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}}')

        query = 'test'
        result = db.query(query)
        assert result.num_rows == 1

    def test_extra_columns_with_none(self):
        """Test that extra_columns=None works."""
        from rhydb import Database

        db = Database()
        create_nucleotide_sequence_table(db,
            table_name="test",
            primary_key_name="id",
            sequence_name="seq",
            reference_sequence="ACGT",
            extra_columns=None
        )
        db.append_data_from_string("test", '{"id": "s1", "seq": {"sequence": "AAAA", "insertions": []}}')

        query = 'test'
        result = db.query(query)
        assert result.num_rows == 1

    def test_extra_columns_invalid_type_raises(self):
        """Test that a non-string extra column is rejected by create_table's validation."""
        from rhydb import Database

        db = Database()
        with pytest.raises(ValueError, match="non-empty string 'name'"):
            create_nucleotide_sequence_table(db,
                table_name="test",
                primary_key_name="id",
                sequence_name="seq",
                reference_sequence="ACGT",
                extra_columns=["valid", 123]  # 123 is not a string
            )

    def test_gene_table_with_extra_columns(self):
        """Test creating a gene table with extra columns."""
        from rhydb import Database

        db = Database()
        create_gene_table(db,
            table_name="genes",
            primary_key_name="id",
            gene_name="spike",
            reference_sequence="MFVFLVLLPLVSSQCVNLTTRTQLPPAYTNSFTRGVYYPDKVFRSSVLHSTQDLFLPFFSNVTWFHAI*",
            extra_columns=["variant", "source"]
        )
        # If no exception, table was created
