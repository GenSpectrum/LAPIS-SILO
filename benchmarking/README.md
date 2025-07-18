# Makefile / scripts to do SILO benchmarking runs

This is meant to be run via the `evobench-run` tool from
[evobench](https://github.com/GenSpectrum/evobench/).

 1. Make sure you have the Rust compiler, version 1.86.0 or later,
    installed: get it via [rustup.rs](https://rustup.rs/) (if you
    already installed the compiler earlier, but you get a message
    about async closures not being stable, then the compiler is too
    old, you can then update it via `rustup update` (or, if you want
    the oldest supported version, `rustup toolchain install 1.86.0`
    then `rustup toolchain list` then `rustup default <the name in the
    list>`)

 1. Install `evobench-run` by running:
 
        cargo install --locked https://github.com/GenSpectrum/evobench/

 1. Set up configuration for `evobench-run`, currently at
    `~/.evobench-run.ron` (if you need configuration in a local
    directory, please submit a feature request); copy the included
    file [evobench-run.ron](evobench-run.ron) and adapt it to your
    local paths and wishes. For local (interactive) use, you will want
    to make sure `benchmarking_job_settings.error_budget` is set to 1,
    since chances are high that you have a failure preventing the
    benchmark from running, and you don't want the benchmarking
    process to re-try. You may also want to change
    `custom_parameters_set` to only use one `CustomParameters`.
    
    (While RON works best to represent the types referred to in the
    configuration, other config file formats are supported, too--run
    `evobench-run config-formats` to see which, and `evobench-run
    config-save` to recode an already-placed file.)

 1. Make sure that you have a folder
    `~/silo-benchmark-datasets/$DATASET` (where
    `$DATASET` is the value of the
    `DATASET` setting in [the config
    file](evobench-run.ron) that you use), with these files (or symlinks to them):
    
        database_config.yaml
        input_file.ndjson.zst
        lineage_definitions.yaml
        reference_genomes.json
        silo_queries.ndjson

    Currently, get these files from gs-staging-1, or when running
    there, use the folder `~christian/silo-benchmark-datasets` (it is
    planned to fetch these via S3 in the future).

 1. Run an instance of a daemon, `evobench-run --verbose run daemon`
    (the `--verbose` allows you to see what's going on, feel free to
    omit it). Important: since the daemon is going to build SILO, and
    neither the SILO build process nor the setup here install conan
    for you, you need to make sure that conan is in the context of the
    daemon. In other words, run your `source ~/venv/bin/activate` or
    similar *before* starting the daemon.

 1. Insert some jobs. If you're working locally, to benchmark the
    current commit, run `evobench-run insert-local HEAD` from within
    the directory that is configured as the `remote_repository.url` in
    `~/.evobench-run.ron`.

 1. Observe the status of your jobs via `evobench-run list`. (Use
    `evobench-run list-all` to see which jobs were inserted when.) If
    you need to inspect errors, `cd
    ~/.evobench-run/working_directory_pool/` then inspect the
    `*.error_at*processing_error file` and if necessary the contents
    of the associated working directory. Once done with inspection,
    you can make the working directory available for re-use by
    renaming it to just the id at the beginning of the directory name
    (unless perhaps it's somehow messed up in a way that "git reset
    --hard" won't recover from, i.e. .git's metadata is broken, or
    there are issues in the SILO build system that would require a
    make clean or similar--in those cases delete the directory).

 1. Once the jobs have finished, find the results in
    `~/silo-benchmark-outputs/` (or whatever directory you have
    changed `output_base_dir` to).

