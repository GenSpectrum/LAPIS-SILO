"""Shared paths and table-creation helpers for the rhydb Python tests."""

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


def _register_reference(db, name, reference):
    """Register a sequence column's reference in the built-in ``reference_genomes`` table.

    The generic ``create_table`` interface resolves sequence-column references from the built-in
    ``reference_genomes`` table rather than inline, so the reference must be present before the
    sequence table is created. Uses the ``register_reference`` short-hand.
    """
    db.register_reference(name, reference)


def create_nucleotide_sequence_table(
    db, table_name, primary_key_name, sequence_name, reference_sequence, extra_columns=None
):
    """Create a nucleotide sequence table through the generic ``create_table`` interface.
    """
    _register_reference(db, sequence_name, reference_sequence)
    columns = [{"name": primary_key_name, "type": "string"}]
    columns += [{"name": col, "type": "string"} for col in (extra_columns or [])]
    columns.append({"name": sequence_name, "type": "nucleotide_sequence"})
    db.create_table(table_name, columns)


def create_gene_table(
    db, table_name, primary_key_name, gene_name, reference_sequence, extra_columns=None
):
    """Create an amino acid sequence table through the generic ``create_table`` interface.
    """
    _register_reference(db, gene_name, reference_sequence)
    columns = [{"name": primary_key_name, "type": "string"}]
    columns += [{"name": col, "type": "string"} for col in (extra_columns or [])]
    columns.append({"name": gene_name, "type": "amino_acid_sequence"})
    db.create_table(table_name, columns)
