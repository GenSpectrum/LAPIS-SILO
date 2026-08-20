"""Tests for saving a database checkpoint and loading it back."""

import os

from .helpers import INPUT_FILE, create_nucleotide_sequence_table


class TestSaveAndLoadCheckpoint:
    """Test saving and loading database checkpoints."""

    def test_save_checkpoint(self, empty_database, main_reference_sequence, temp_dir):
        """Test saving a database checkpoint."""
        create_nucleotide_sequence_table(empty_database,
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
        from rhydb import Database

        # Create and save
        create_nucleotide_sequence_table(empty_database,
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
        from rhydb import Database

        # Create, add data, and save
        create_nucleotide_sequence_table(empty_database,
            table_name="sequences",
            primary_key_name="primary_key",
            sequence_name="main",
            reference_sequence=main_reference_sequence
        )
        empty_database.append_data_from_file("sequences", INPUT_FILE)

        # Get data before save
        bitmap_before = empty_database.get_filtered_bitmap("sequences", 'true')

        # Save and reload
        save_path = os.path.join(temp_dir, "checkpoint")
        empty_database.save_checkpoint(save_path)

        loaded_db = Database(save_path)

        # Compare with loaded data
        bitmap_after = loaded_db.get_filtered_bitmap("sequences", 'true')

        assert bitmap_before == bitmap_after
