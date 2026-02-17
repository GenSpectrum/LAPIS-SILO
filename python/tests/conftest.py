import pytest
import tempfile
import shutil
import os


@pytest.fixture
def temp_dir():
    """Create a temporary directory for test files."""
    temp_path = tempfile.mkdtemp()
    yield temp_path
    shutil.rmtree(temp_path, ignore_errors=True)


@pytest.fixture
def empty_database():
    """Create an empty database instance."""
    from silodb import Database
    return Database()
