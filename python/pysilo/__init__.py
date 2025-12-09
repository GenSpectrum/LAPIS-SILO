"""
PySilo - Python bindings for Silo C++ database library.

PySilo provides a Pythonic interface to the Silo high-performance
database system for genomic and sequence data.
"""

from .column_type import ColumnType
from .column_identifier import ColumnIdentifier
from .database import Database

__all__ = ['Database', 'ColumnIdentifier', 'ColumnType']
__version__ = '0.1.0'
