"""Shared paths and table-creation helpers for the rhydb Python tests."""

import json
import os

# Path to test data
TEST_DATA_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'testBaseData', 'exampleDataset')
REFERENCE_GENOMES_FILE = os.path.join(TEST_DATA_DIR, 'reference_genomes.json')
INPUT_FILE = os.path.join(TEST_DATA_DIR, 'input_file.ndjson')

# A serialized database (checked into the repo) whose schema has scalar value columns
# (age:int, qc_value:float, date:date, test_boolean_column:bool). Scalar columns can now also be
# created directly through create_table; loading this state additionally exercises the on-disk
# load path for such columns.
SERIALIZED_STATE_DIR = os.path.join(
    os.path.dirname(__file__), '..', '..', 'testBaseData', 'siloSerializedState'
)


def declare_column_reference(db, table_name, column_name, column_type, reference_name):
    """Declare in ``reference_columns`` which reference backs a column of ``table_name``.

    ``create_table`` reads this mapping and never writes it, so a column of a type that needs a
    reference must have its row appended first. The row goes in through the ordinary append path,
    and its ``id`` has to be ``"<table_name>.<column_name>"``.
    """
    db.append_data_from_string(
        "reference_columns",
        json.dumps(
            {
                "id": f"{table_name}.{column_name}",
                "table_name": table_name,
                "column_name": column_name,
                "column_type": column_type,
                "reference_name": reference_name,
            }
        ),
    )


def _create_sequence_table(
    db, table_name, primary_key_name, sequence_name, reference_sequence, column_type, extra_columns
):
    """Register a reference, declare the column it backs, and create the table.

    The three steps a sequence table now takes: the reference goes into ``reference_genomes``, the
    mapping into ``reference_columns``, and only then can the table be created.
    """
    db.register_reference(sequence_name, reference_sequence)
    declare_column_reference(db, table_name, sequence_name, column_type, sequence_name)
    columns = [{"name": primary_key_name, "type": "string"}]
    columns += [{"name": col, "type": "string"} for col in (extra_columns or [])]
    columns.append({"name": sequence_name, "type": column_type})
    db.create_table(table_name, columns)


def create_nucleotide_sequence_table(
    db, table_name, primary_key_name, sequence_name, reference_sequence, extra_columns=None
):
    """Create a nucleotide sequence table through the generic ``create_table`` interface."""
    _create_sequence_table(
        db,
        table_name,
        primary_key_name,
        sequence_name,
        reference_sequence,
        "nucleotide_sequence",
        extra_columns,
    )


def create_gene_table(
    db, table_name, primary_key_name, gene_name, reference_sequence, extra_columns=None
):
    """Create an amino acid sequence table through the generic ``create_table`` interface."""
    _create_sequence_table(
        db,
        table_name,
        primary_key_name,
        gene_name,
        reference_sequence,
        "amino_acid_sequence",
        extra_columns,
    )
