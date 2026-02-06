# python/pysilo/database.pyx
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.optional cimport optional
from libc.stdint cimport uint64_t, uint32_t
from libc.stdlib cimport malloc, free
from cython.operator cimport dereference as deref
from database cimport Database as CppDatabase, Roaring as CppRoaring
import os
import pyroaring
import pyarrow as pa

cdef class PyDatabase:
    """Python wrapper for C++ Database"""
    cdef CppDatabase* c_database
    
    def __cinit__(self, str file_name=None):
        cdef string cpp_file_name
        cdef optional[CppDatabase] loaded

        if file_name is not None:
            if not os.path.exists(file_name):
                raise FileNotFoundError(f"Database file not found: {file_name}")

            cpp_file_name = file_name.encode('utf-8')
            loaded = CppDatabase.loadDatabaseStateFromPath(cpp_file_name)

            if not loaded.has_value():
                raise RuntimeError(f"Failed to load database from '{file_name}'")

            # Allocate and move the loaded database to heap
            self.c_database = new CppDatabase(deref(loaded))
        else:
            self.c_database = new CppDatabase()
    
    def __dealloc__(self):
        if self.c_database != NULL:
            del self.c_database

    def save_checkpoint(self, str save_directory):
        """
        Save a checkpoint of the database to a directory

        Parameters
        ----------
        save_directory : str
            Path to the directory where the database state will be saved
        """
        if not save_directory or not save_directory.strip():
            raise ValueError("save_directory cannot be empty")

        cdef string cpp_save_directory = save_directory.encode('utf-8')

        try:
            self.c_database.saveDatabaseState(cpp_save_directory)
        except Exception as e:
            raise RuntimeError(f"Failed to save checkpoint to '{save_directory}': {e}")

    def get_tables(self):
        """
        Returns a list of all tables in this database

        Returns
        -------
        pyarrow.Table
            pyarrow.Table with a 'table_name' column containing all table names

        """
        cdef string ipc_buffer

        try:
            ipc_buffer = self.c_database.getTablesAsArrowIpc()

            # Convert IPC buffer to PyArrow Table
            ipc_bytes = (<char*> ipc_buffer.data())[:ipc_buffer.size()]
            buffer_reader = pa.BufferReader(ipc_bytes)
            reader = pa.ipc.open_stream(buffer_reader)
            return reader.read_all()
        except Exception as e:
            raise RuntimeError(f"Failed to get tables: {e}")

    
    def create_nucleotide_sequence_table(self, str table_name, str primary_key_name, str sequence_name, str reference_sequence, list extra_columns=None):
        """
        Create a new nucleotide sequence table

        Parameters
        ----------
        table_name : str
            Name of the table
        primary_key_name : str
            Name of the primary key column
        sequence_name : str
            Name of the nucleotide sequence column
        reference_sequence : str
            The reference nucleotide sequence (e.g., "ACGT...")
        extra_columns : list of str, optional
            Additional string columns to add to the table (default: None)
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not primary_key_name or not primary_key_name.strip():
            raise ValueError("primary_key_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")
        if not reference_sequence or not reference_sequence.strip():
            raise ValueError("reference_sequence cannot be empty")

        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_primary_key_name = primary_key_name.encode('utf-8')
        cdef string cpp_sequence_name = sequence_name.encode('utf-8')
        cdef string cpp_reference_sequence = reference_sequence.encode('utf-8')
        cdef vector[string] cpp_extra_columns

        if extra_columns:
            for col in extra_columns:
                if not isinstance(col, str):
                    raise TypeError(f"extra_columns must contain strings, got {type(col)}")
                cpp_extra_columns.push_back(col.encode('utf-8'))

        try:
            self.c_database.createNucleotideSequenceTable(cpp_table_name, cpp_primary_key_name, cpp_sequence_name, cpp_reference_sequence, cpp_extra_columns)
        except Exception as e:
            raise RuntimeError(f"Failed to create table '{table_name}': {e}")

    def create_gene_table(self, str table_name, str primary_key_name, str gene_name, str reference_sequence, list extra_columns=None):
        """
        Create a new gene (amino acid sequence) table

        Parameters
        ----------
        table_name : str
            Name of the table
        primary_key_name : str
            Name of the primary key column
        gene_name : str
            Name of the amino acid sequence column
        reference_sequence : str
            The reference amino acid sequence
        extra_columns : list of str, optional
            Additional string columns to add to the table (default: None)
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not primary_key_name or not primary_key_name.strip():
            raise ValueError("primary_key_name cannot be empty")
        if not gene_name or not gene_name.strip():
            raise ValueError("gene_name cannot be empty")
        if not reference_sequence or not reference_sequence.strip():
            raise ValueError("reference_sequence cannot be empty")

        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_primary_key_name = primary_key_name.encode('utf-8')
        cdef string cpp_gene_name = gene_name.encode('utf-8')
        cdef string cpp_reference_sequence = reference_sequence.encode('utf-8')
        cdef vector[string] cpp_extra_columns

        if extra_columns:
            for col in extra_columns:
                if not isinstance(col, str):
                    raise TypeError(f"extra_columns must contain strings, got {type(col)}")
                cpp_extra_columns.push_back(col.encode('utf-8'))

        try:
            self.c_database.createGeneTable(cpp_table_name, cpp_primary_key_name, cpp_gene_name, cpp_reference_sequence, cpp_extra_columns)
        except Exception as e:
            raise RuntimeError(f"Failed to create table '{table_name}': {e}")
    
    def append_data_from_file(self, str table_name, str file_name):
        """
        Append data from file to table
        
        Parameters
        ----------
        table_name : str
            Name of the table
        file_name : str
            Path to data file
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        
        if not file_name or not file_name.strip():
            raise ValueError("file_name cannot be empty")
        
        if not os.path.exists(file_name):
            raise FileNotFoundError(f"File not found: {file_name}")
        
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_file_name = file_name.encode('utf-8')
        
        try:
            self.c_database.appendDataFromFile(cpp_table_name, cpp_file_name)
        except Exception as e:
            raise RuntimeError(
                f"""Failed to append data: {e}
                The database object might be invalid now. Any further operations can lead to undefined behavior."""
            )

    def append_data_from_string(self, str table_name, str json_string):
        """
        Append data from a JSON string to table
        
        Parameters
        ----------
        table_name : str
            Name of the table
        json_string : str
            a serialized json containing the data to append
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        
        if not json_string or not json_string.strip():
            raise ValueError("json_string cannot be empty")
        
        cdef string cpp_table_name = table_name.encode('utf-8')
        cdef string cpp_json_string = json_string.encode('utf-8')
        
        try:
            self.c_database.appendDataFromString(cpp_table_name, cpp_json_string)
        except Exception as e:
            raise RuntimeError(
                f"""Failed to append data: {e}
                The database object might be invalid now. Any further operations can lead to undefined behavior."""
            )
    def print_all_data(self, str table_name):
        """
        Print all data in a table to stdout

        Parameters
        ----------
        table_name : str
            Name of the table
        """
        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")

        cdef string cpp_table_name = table_name.encode('utf-8')

        try:
            self.c_database.printAllData(cpp_table_name)
        except Exception as e:
            raise RuntimeError(f"Failed to print data from table '{table_name}': {e}")

    def get_nucleotide_reference_sequence(self, str table_name, str sequence_name):
        """
        Get the nucleotide reference sequence for a given table and sequence name

        Parameters
        ----------
        table_name : str
            Name of the table
        sequence_name : str
            Name of the sequence

        Returns
        -------
        str
            The nucleotide reference sequence
        """
        cdef string cpp_table_name
        cdef string cpp_sequence_name
        cdef string result

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")

        cpp_table_name = table_name.encode('utf-8')
        cpp_sequence_name = sequence_name.encode('utf-8')

        try:
            result = self.c_database.getNucleotideReferenceSequence(cpp_table_name, cpp_sequence_name)
            return result.decode('utf-8')
        except Exception as e:
            raise RuntimeError(f"Failed to get nucleotide reference sequence: {e}")

    def get_amino_acid_reference_sequence(self, str table_name, str sequence_name):
        """
        Get the amino acid reference sequence for a given table and sequence name

        Parameters
        ----------
        table_name : str
            Name of the table
        sequence_name : str
            Name of the sequence

        Returns
        -------
        str
            The nucleotide reference sequence
        """
        cdef string cpp_table_name
        cdef string cpp_sequence_name
        cdef string result

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")

        cpp_table_name = table_name.encode('utf-8')
        cpp_sequence_name = sequence_name.encode('utf-8')

        try:
            result = self.c_database.getAminoAcidReferenceSequence(cpp_table_name, cpp_sequence_name)
            return result.decode('utf-8')
        except Exception as e:
            raise RuntimeError(f"Failed to get amino acid reference sequence: {e}")

    def get_prevalent_nucleotide_mutations(self, str table_name, str sequence_name, double prevalence_threshold, str filter_expression=""):
        """
        Get prevalent mutations above a threshold

        Parameters
        ----------
        table_name : str
            Name of the table
        sequence_name : str
            Name of the sequence
        prevalence_threshold : float
            Minimum prevalence threshold (0.0 to 1.0)
        filter_expression : str, optional
            Filter expression to apply (default: "")

        Returns
        -------
        list of tuple
            List of (position, mutation) tuples where position is an int
            and mutation is a string
        """
        cdef string cpp_table_name
        cdef string cpp_sequence_name
        cdef string cpp_filter
        cdef vector[pair[uint64_t, string]] mutations

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")
        if prevalence_threshold < 0.0 or prevalence_threshold > 1.0:
            raise ValueError("prevalence_threshold must be between 0.0 and 1.0")
        # Default to True filter (returns all rows) if no filter specified
        if filter_expression is None or filter_expression == "":
            filter_expression = '{"type":"True"}'

        cpp_table_name = table_name.encode('utf-8')
        cpp_sequence_name = sequence_name.encode('utf-8')
        cpp_filter = filter_expression.encode('utf-8')

        try:
            mutations = self.c_database.getPrevalentNucMutations(
                cpp_table_name,
                cpp_sequence_name,
                prevalence_threshold,
                cpp_filter
            )

            result = []
            for i in range(mutations.size()):
                result.append((mutations[i].first, mutations[i].second.decode('utf-8')))

            return result
        except ValueError:
            raise
        except Exception as e:
            raise RuntimeError(f"Failed to get prevalent mutations: {e}")

    def get_prevalent_amino_acid_mutations(self, str table_name, str sequence_name, double prevalence_threshold, str filter_expression=""):
        """
        Get prevalent mutations above a threshold

        Parameters
        ----------
        table_name : str
            Name of the table
        sequence_name : str
            Name of the sequence
        prevalence_threshold : float
            Minimum prevalence threshold (0.0 to 1.0)
        filter_expression : str, optional
            Filter expression to apply (default: "")

        Returns
        -------
        list of tuple
            List of (position, mutation) tuples where position is an int
            and mutation is a string
        """
        cdef string cpp_table_name
        cdef string cpp_sequence_name
        cdef string cpp_filter
        cdef vector[pair[uint64_t, string]] mutations

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not sequence_name or not sequence_name.strip():
            raise ValueError("sequence_name cannot be empty")
        if prevalence_threshold < 0.0 or prevalence_threshold > 1.0:
            raise ValueError("prevalence_threshold must be between 0.0 and 1.0")
        # Default to True filter (returns all rows) if no filter specified
        if filter_expression is None or filter_expression == "":
            filter_expression = '{"type":"True"}'

        cpp_table_name = table_name.encode('utf-8')
        cpp_sequence_name = sequence_name.encode('utf-8')
        cpp_filter = filter_expression.encode('utf-8')

        try:
            mutations = self.c_database.getPrevalentAminoAcidMutations(
                cpp_table_name,
                cpp_sequence_name,
                prevalence_threshold,
                cpp_filter
            )

            result = []
            for i in range(mutations.size()):
                result.append((mutations[i].first, mutations[i].second.decode('utf-8')))

            return result
        except ValueError:
            raise
        except Exception as e:
            raise RuntimeError(f"Failed to get prevalent mutations: {e}")

    def get_filtered_bitmap(self, str table_name, str filter_expression=""):
        """
        Get a roaring bitmap of row indices matching the filter expression

        Parameters
        ----------
        table_name : str
            Name of the table
        filter_expression : str, optional
            Filter expression in JSON format (default: '{"type":"True"}' which matches all rows)

        Returns
        -------
        pyroaring.BitMap
            A roaring bitmap containing the indices of matching rows
        """
        cdef string cpp_table_name
        cdef string cpp_filter
        cdef CppRoaring bitmap
        cdef size_t size_in_bytes
        cdef char* buffer

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")

        # Default to True filter (returns all rows) if no filter specified
        if filter_expression is None or filter_expression == "":
            filter_expression = '{"type":"True"}'

        cpp_table_name = table_name.encode('utf-8')
        cpp_filter = filter_expression.encode('utf-8')

        try:
            bitmap = self.c_database.getFilteredBitmap(cpp_table_name, cpp_filter)

            # Serialize the C++ bitmap to bytes
            size_in_bytes = bitmap.getSizeInBytes()
            buffer = <char*>malloc(size_in_bytes)
            if buffer == NULL:
                raise MemoryError("Failed to allocate buffer for bitmap serialization")

            try:
                bitmap.write(buffer)
                # Create Python bytes object from buffer
                serialized = bytes(buffer[:size_in_bytes])
            finally:
                free(buffer)

            # Deserialize into pyroaring BitMap
            return pyroaring.BitMap.deserialize(serialized)
        except ValueError:
            raise
        except Exception as e:
            raise RuntimeError(f"Failed to get filtered bitmap: {e}")

    def execute_query(self, str table_name, str query_json):
        """
        Execute a query and return results as a PyArrow Table

        Parameters
        ----------
        table_name : str
            Name of the table to query
        query_json : str
            Query in JSON format. Must contain 'filterExpression' and 'action' fields.
            Example: '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'

        Returns
        -------
        pyarrow.Table
            Query results as a PyArrow Table. Use .to_batches() to get individual RecordBatches.

        Example
        -------
        >>> db = PyDatabase("path/to/database")
        >>> query = '{"filterExpression": {"type": "True"}, "action": {"type": "Details"}}'
        >>> table = db.execute_query("my_table", query)
        >>> print(table.schema)
        >>> df = table.to_pandas()  # Convert to pandas DataFrame
        """
        cdef string cpp_table_name
        cdef string cpp_query_json
        cdef string ipc_buffer

        if not table_name or not table_name.strip():
            raise ValueError("table_name cannot be empty")
        if not query_json or not query_json.strip():
            raise ValueError("query_json cannot be empty")

        cpp_table_name = table_name.encode('utf-8')
        cpp_query_json = query_json.encode('utf-8')

        try:
            ipc_buffer = self.c_database.executeQueryAsArrowIpc(cpp_table_name, cpp_query_json)

            # Convert IPC buffer to PyArrow Table
            buffer_reader = pa.BufferReader(ipc_buffer)
            reader = pa.ipc.open_stream(buffer_reader)
            return reader.read_all()
        except ValueError:
            raise
        except Exception as e:
            raise RuntimeError(f"Failed to execute query: {e}")

    def __repr__(self):
        return "Database()"
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        return False