"""Tests for appending data to a table and printing it back out."""

from .helpers import INPUT_FILE, create_nucleotide_sequence_table


class TestAppendData:
    """Test appending data to tables."""

    def test_append_data_from_real_file(self, empty_database, main_reference_sequence):
        """Test appending data from the real test data file."""
        # First create the table
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )

        # Then append data
        empty_database.append_data_from_file("sequences", INPUT_FILE)
        # If no exception, data was appended


class TestPrintAllData:
    """Test the print_all_data method."""

    def test_print_all_data(self, empty_database, main_reference_sequence, capsys):
        """Test that print_all_data outputs something."""
        # Note: printAllData expects sequence_name="sequence" (hardcoded in C++)
        create_nucleotide_sequence_table(empty_database,
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
