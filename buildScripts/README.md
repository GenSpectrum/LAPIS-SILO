# buildScripts

Helper scripts invoked by the root `Makefile`.

## uv

[uv](https://docs.astral.sh/uv/) is required to run these scripts. It manages the Python environment
used to install Conan (see `pyproject.toml`). The scripts call `uv run --project buildScripts`,
which uses the locked environment defined here rather than any currently active virtual environment.
`uv` caches downloaded packages outside the project in a user-global directory
(`~/.cache/uv` on Linux, `~/Library/Caches/uv` on macOS). See
[uv dependency caching](https://docs.astral.sh/uv/concepts/cache/#dependency-caching).
