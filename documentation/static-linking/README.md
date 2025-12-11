# Attempts at getting a statically linked SILO binary

Here's the most essential parts of the pursued paths, each file a separate one.

Given that all paths have failed, and need considerable more work, and probably will cause considerable pain to keep working into the future, it will be best to give up on static linking for the current use range and instead link with glibc dynamically, but do so on an older system (Debian oldstable), to increase the range of Linux systems that the resulting binary is compatible with.

    buster   (10.0) 2019-07-06
    bullseye (11.0) 2021-08-14  -- oldoldstable
    bookworm (12.0) 2023-06-10  -- oldstable
    trixie   (13.0) 2025-08-09  -- stable
    sid      (14.0) 

bookworm is the version to choose: it's the version staging is running on, so we know SILO works. bullseye has just about 8 months of security support left, and its compilers are probably too old for SILO.
