"""PySilo - Python bindings for Silo"""

from importlib.metadata import version, PackageNotFoundError

from .database import PyDatabase as Database

__all__ = ['Database']

try:
    __version__ = version("pysilo")
except PackageNotFoundError:
    __version__ = "0.0.0"  # Fallback for development