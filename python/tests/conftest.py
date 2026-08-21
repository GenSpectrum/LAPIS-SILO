import json
import shutil
import tempfile

import pytest

from .helpers import REFERENCE_GENOMES_FILE


@pytest.fixture
def temp_dir():
    """Create a temporary directory for test files."""
    temp_path = tempfile.mkdtemp()
    yield temp_path
    shutil.rmtree(temp_path, ignore_errors=True)


@pytest.fixture
def empty_database():
    """Create an empty database instance."""
    from rhydb import Database
    return Database()


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
