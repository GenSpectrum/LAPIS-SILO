# Sequence Storage

This document explains the fundamental concepts of how sequences are stored in SILO.

## Overview

SILO uses a hybrid storage approach that separates:
- **Columnar storage** for mutations (differences from reference sequences)
- **Row-wise storage** for coverage data (location of the N symbols in nucleotides or X symbols in amino acids)

Both storage types use bitmaps, but we provide separate data structures holding the respective Roaring bitmap containers. This separation enables optimizations for both storage efficiency and query performance.

These special structures are introduced to allow efficient operations on the sequence data with minimal storage requirements.
In particular, we do not want to store entire bitmaps for all positions, because these can be mostly empty. It is still advantageous to keep bitmap containers for all variation in the sequence as this still allows very fast computations.
The containers are organized in a tree-map for fast iteration. 

## Roaring Bitmap Containers

A Roaring bitmap container represents a bitmap with a domain of `[0, 2^16)` (0 to 65,535).

The Roaring library provides `roaring::Roaring`, which is an array of Roaring containers that also stores the offset for each container. This allows it to represent the full 32-bit domain.

However, individual containers can be used separately to enable more low-level optimizations and storage-aware implementations of data structures and algorithms.

## Vertical Sequence Index

We conceptually divide the sequence space into tiles with a side length of 2^16 (65,536):
```
        0      2^16   2*2^16 3*2^16
        |      |      |      |
      0-┌──────┬──────┬──────┬──────┬────>
        │      │      │      │      │  Position (x-axis)
        │ Tile │ Tile │ Tile │ Tile │
        │ 0,0  │ 0,1  │ 0,2  │ 0,3  │
   2^16-├──────┼──────┼──────┼──────┼
        │      │      │      │      │
        │ Tile │ Tile │ Tile │ Tile │
        │ 1,0  │ 1,1  │ 1,2  │ 1,3  │
 2*2^16-├──────┼──────┼──────┼──────┼
        │      │      │      │      │
        │ Tile │ Tile │ Tile │ Tile │
        │ 2,0  │ 2,1  │ 2,2  │ 2,3  │
        ├──────┼──────┼──────┼──────┼
        │
        v
    Sequence Number (y-axis)
```

### Tiling Structure

**Horizontal tiling (x-axis)**: Identified by genome position
- The conceptual tile number is `position_id / 2^16`
- This is not stored explicitly

**Vertical tiling (y-axis)**: Identified explicitly by `v_index`
- The bitmap for a single position is split across tiles of size 2^16
- Each tile's offset is identified by its `v_index`
- For example, at position 10, we can have multiple bitmaps:
    - One covering sequence range `[0, 2^16)`
    - One for range `[2^16, 2*2^16)`
    - One for range `[2*2^16, 3*2^16)`
    - And so on...

### Lookup Process

To look up position `x` of sequence `y` and symbol `z`:

1. Identify the vertical tile: `v_index = y / 2^16`
2. Find the bitmap for position `x` with `v_index` and symbol `z`
3. Look up index `y % 2^16` within that bitmap container

### Storage Organization

For every genome position, we store the differences (mutations) from the reference sequence in vertical bitmap containers, with one container per symbol (A, C, G, T, etc.).

These bitmap containers are organized in a tree structure for efficient iteration and lookup. They are sorted by the key: `{position, v_index, symbol}`.
