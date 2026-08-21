"""Tests for create_table and the sequence/gene table helpers built on top of it."""

import json

import pytest

from .helpers import (
    create_gene_table,
    create_nucleotide_sequence_table,
    declare_column_reference,
)


class TestCreateTable:
    """Test the generic create_table method that accepts columns of any supported type."""

    @staticmethod
    def _populate_references(db, table_name, entries):
        """Register references and declare that they back the same-named columns of `table_name`.

        `entries` maps a sequence column name to (reference string, column type).
        """
        for name, (reference, column_type) in entries.items():
            db.register_reference(name, reference)
            declare_column_reference(db, table_name, name, column_type, name)

    def test_create_table_with_all_column_types(self):
        """A table can be created with every supported column type and populated."""
        from rhydb import Database

        db = Database()
        self._populate_references(
            db,
            "samples",
            {"seq": ("ACGT", "nucleotide_sequence"), "gene": ("MFV", "amino_acid_sequence")},
        )
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
        # Each sequence column's reference came from its `reference_columns` declaration.
        assert db.get_nucleotide_reference_sequence("samples", "seq") == "ACGT"
        assert db.get_amino_acid_reference_sequence("samples", "gene") == "MFV"

    def test_create_table_reference_taken_from_the_declaration(self):
        """A sequence column's reference is the one its `reference_columns` row names."""
        from rhydb import Database

        db = Database()
        self._populate_references(db, "sequences", {"main": ("ACGTACGT", "nucleotide_sequence")})
        db.create_table(
            "sequences",
            [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
        )

        assert db.get_nucleotide_reference_sequence("sequences", "main") == "ACGTACGT"

    def test_create_table_undeclared_sequence_column_raises(self):
        """A sequence column with no `reference_columns` row declaring its reference fails."""
        from rhydb import Database

        db = Database()
        self._populate_references(db, "samples", {"other": ("ACGT", "nucleotide_sequence")})
        with pytest.raises(RuntimeError, match="the 'reference_columns' table declares none for it"):
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


class TestRegisterReference:
    """Test register_reference and the uniqueness of reference names."""

    def test_register_reference_duplicate_name_raises(self, empty_database):
        """A second reference of the same name is rejected rather than silently shadowing."""
        empty_database.register_reference("main", "ACGT")
        with pytest.raises(RuntimeError, match="already holds a reference of that name"):
            empty_database.register_reference("main", "TTTTTTTT")

    def test_register_reference_duplicate_name_keeps_first_reference(self, empty_database):
        """The rejected registration leaves the original entry intact."""
        empty_database.register_reference("main", "ACGT")
        with pytest.raises(RuntimeError):
            empty_database.register_reference("main", "TTTTTTTT")

        declare_column_reference(
            empty_database, "samples", "main", "nucleotide_sequence", "main"
        )
        empty_database.create_table(
            "samples",
            [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
        )
        assert empty_database.get_nucleotide_reference_sequence("samples", "main") == "ACGT"

    def test_register_reference_with_matching_type(self, empty_database):
        """An explicit sequence_type that matches the column type is accepted."""
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(empty_database, "samples", "main", "nucleotide_sequence", "main")
        empty_database.create_table(
            "samples",
            [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
        )
        assert empty_database.get_nucleotide_reference_sequence("samples", "main") == "ACGT"

    def test_register_reference_with_mismatched_type_raises(self, empty_database):
        """A reference registered as one sequence kind cannot back a column of the other."""
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(empty_database, "samples", "main", "amino_acid_sequence", "main")
        with pytest.raises(RuntimeError, match="is a 'nucleotide_sequence' reference"):
            empty_database.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "main", "type": "amino_acid_sequence"}],
            )

    def test_register_reference_distinct_names(self, empty_database):
        """Distinct names are accepted and both remain resolvable."""
        empty_database.register_reference("main", "ACGT")
        empty_database.register_reference("gene", "MFV")
        declare_column_reference(empty_database, "samples", "main", "nucleotide_sequence", "main")
        declare_column_reference(empty_database, "samples", "gene", "amino_acid_sequence", "gene")
        empty_database.create_table(
            "samples",
            [
                {"name": "id", "type": "string"},
                {"name": "main", "type": "nucleotide_sequence"},
                {"name": "gene", "type": "amino_acid_sequence"},
            ],
        )
        assert empty_database.get_nucleotide_reference_sequence("samples", "main") == "ACGT"
        assert empty_database.get_amino_acid_reference_sequence("samples", "gene") == "MFV"


class TestColumnReferences:
    """Test the `reference_columns` mapping, appended manually before create_table."""

    def test_column_can_use_a_reference_of_another_name(self, empty_database):
        """A column need not be named after its reference; the declaration links the two."""
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(
            empty_database, "samples", "sequence", "nucleotide_sequence", "main"
        )
        empty_database.create_table(
            "samples",
            [
                {"name": "id", "type": "string"},
                {"name": "sequence", "type": "nucleotide_sequence"},
            ],
        )
        assert empty_database.get_nucleotide_reference_sequence("samples", "sequence") == "ACGT"

    def test_two_columns_can_share_one_reference(self, empty_database):
        """An aligned and an unaligned column back onto a single registered reference."""
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(empty_database, "samples", "main", "nucleotide_sequence", "main")
        declare_column_reference(
            empty_database, "samples", "unaligned_main", "zstd_compressed_string", "main"
        )
        empty_database.create_table(
            "samples",
            [
                {"name": "id", "type": "string"},
                {"name": "main", "type": "nucleotide_sequence"},
                {"name": "unaligned_main", "type": "zstd_compressed_string"},
            ],
        )
        # One reference entry, two columns pointing at it.
        references = empty_database.query("reference_genomes").to_pydict()
        assert references["name"] == ["main"]

        mapping = empty_database.query("reference_columns").to_pydict()
        assert set(mapping["column_name"]) == {"main", "unaligned_main"}
        assert set(mapping["reference_name"]) == {"main"}

    def test_reference_columns_is_a_normal_queryable_table(self, empty_database):
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(empty_database, "samples", "main", "nucleotide_sequence", "main")
        empty_database.create_table(
            "samples",
            [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
        )

        mapping = empty_database.query("reference_columns").to_pydict()
        assert mapping["table_name"] == ["samples"]
        assert mapping["column_name"] == ["main"]
        assert mapping["column_type"] == ["nucleotide_sequence"]
        assert mapping["reference_name"] == ["main"]

    def test_two_tables_do_not_share_columns(self, empty_database):
        """Each table's sequence columns are its own, even in one database."""
        empty_database.register_reference("first_ref", "ACGT", "nucleotide_sequence")
        empty_database.register_reference("second_ref", "TTTT", "nucleotide_sequence")
        declare_column_reference(empty_database, "first", "seq", "nucleotide_sequence", "first_ref")
        declare_column_reference(
            empty_database, "second", "seq", "nucleotide_sequence", "second_ref"
        )
        for table in ("first", "second"):
            empty_database.create_table(
                table,
                [{"name": "id", "type": "string"}, {"name": "seq", "type": "nucleotide_sequence"}],
            )

        assert empty_database.get_nucleotide_reference_sequence("first", "seq") == "ACGT"
        assert empty_database.get_nucleotide_reference_sequence("second", "seq") == "TTTT"

    def test_sequence_column_without_a_declaration_raises(self, empty_database):
        """create_table reads the mapping; it will not invent one."""
        empty_database.register_reference("main", "ACGT")
        with pytest.raises(RuntimeError, match="the 'reference_columns' table declares none for it"):
            empty_database.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
            )

    def test_declaration_naming_an_unknown_reference_raises(self, empty_database):
        empty_database.register_reference("main", "ACGT")
        declare_column_reference(empty_database, "samples", "seq", "nucleotide_sequence", "nope")
        with pytest.raises(RuntimeError, match="names the reference 'nope'"):
            empty_database.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "seq", "type": "nucleotide_sequence"}],
            )

    def test_declaration_of_an_incompatible_kind_raises(self, empty_database):
        """A gene's reference cannot back a nucleotide column."""
        empty_database.register_reference("gene", "MFV", "amino_acid_sequence")
        declare_column_reference(empty_database, "samples", "seq", "nucleotide_sequence", "gene")
        with pytest.raises(RuntimeError, match="needs a 'nucleotide_sequence' reference"):
            empty_database.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "seq", "type": "nucleotide_sequence"}],
            )

    def test_declaration_disagreeing_about_the_column_type_raises(self, empty_database):
        empty_database.register_reference("main", "ACGT", "nucleotide_sequence")
        declare_column_reference(
            empty_database, "samples", "main", "zstd_compressed_string", "main"
        )
        with pytest.raises(RuntimeError, match="but it is being created with type"):
            empty_database.create_table(
                "samples",
                [{"name": "id", "type": "string"}, {"name": "main", "type": "nucleotide_sequence"}],
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
