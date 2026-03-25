# Serialization Version

The serialization version is a Unix timestamp (seconds since epoch) that identifies the binary format used to persist the database. It is defined in `src/silo/common/serialization_version.txt` and included into `src/silo/common/data_version.cpp` as `CURRENT_SILO_SERIALIZATION_VERSION`.

## Bumping the serialization version

When you change the serialization format (e.g. adding/removing/reordering fields in persisted data), run the provided script:

```bash
make bump-serialization-version
```

This:
1. Generates a new Unix timestamp.
2. Updates `serialization_version.txt` with the new timestamp.
3. Removes old serialized test state directories.
4. Builds the test binary.
5. Runs the save/reload test to produce a new serialized state directory.

After it finishes, `git add` the new directory under `testBaseData/siloSerializedState/` and commit.
