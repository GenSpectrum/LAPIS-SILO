all: please_give_target_name__api_or_preprocessing

# Required env vars:

# Added by evobench-run, and we rely on them
ifeq ($(COMMIT_ID),)
$(error "COMMIT_ID must be set")
endif
ifeq ($(BENCH_OUTPUT_LOG),)
$(error "BENCH_OUTPUT_LOG must be set")
endif

# Our custom parameters (the shared ones, there can be more checks in
# the `Makefile_*` files):

# The name of a subfolder of the setting in
# `versioned_datasets_base_dir`, where the versioned datasets are
# stored; in spite of `DATASET_DIR` below having the full path, we
# still also use this dataset name, thus require it to be set
# (guaranteed when running via evobench-run, but not when running
# manually).
ifeq ($(DATASET),)
$(error "DATASET must be a subfolder name, got '$(DATASET)'")
endif

# Set by `evobench-run run`--chosen according to the optional
# `versioned_datasets_base_dir` setting in `evobench-run.ron`, the
# value of the `DATASET` custom variable, the current commit id, and
# the names of the subdirectories of `DATASET_DIR` representing git
# revision names. See documentation in the `evobench-run.ron` file,
# and [Offer dataset versioning
# #32](https://github.com/GenSpectrum/evobench/issues/32).
ifeq ($(DATASET_DIR),)
$(error "DATASET_DIR must be the path to the appropriate dataset folder")
endif

# Should sorted input be used?
ifeq ($(SORTED),0)
INPUT_FILE = $(DATASET_DIR)/input_file.ndjson.zst
OUTPUT_DIR ?= $(DATASET)-$(COMMIT_ID)-output
else ifeq ($(SORTED),1)
# OK to write back into the $(DATASET_DIR)?
INPUT_FILE = $(DATASET_DIR)/sorted_input_file.ndjson.zst
OUTPUT_DIR ?= $(DATASET)-$(COMMIT_ID)-sorted_output
else
$(error "SORTED must be either 0 or 1, got '$(SORTED)'")
endif
export INPUT_FILE
export OUTPUT_DIR

# Should a debug build be used?
ifeq ($(DEBUG),)
BUILD=Release
else
BUILD=Debug
endif
export BUILD

# ---- General -----------------------------------------------------------------

SILO=../build/$(BUILD)/silo_$(COMMIT_ID)
export SILO
PREPROCESSING_STAMP = $(OUTPUT_DIR)/.done
export PREPROCESSING_STAMP

$(SILO):
	bin/build-silo

# ---- Benchmarking targets ----------------------------------------------------

# Want data on the preprocessing, hence remove the stamp and let it
# re-run. Do not also run the api since it would clobber the log file
# from the preprocessing.
preprocessing: $(SILO)
	rm -f $(PREPROCESSING_STAMP)
	make -f Makefile_preprocessing

# Want data on the api; still run the preprocessing if needed but
# without logging
api: $(SILO)
	( unset EVOBENCH_LOG; make -f Makefile_preprocessing )
	make -f Makefile_api

# ---- Other targets -----------------------------------------------------------

# just for manual call comfort
dependencies:
	make -f Makefile_preprocessing dependencies
	make -f Makefile_api dependencies

clean:
	rm -rf *-output/ *-sorted_output/ logs sorted_chunks merger_tmp

clean-fully: clean
	rm -rf ../build

# ----------------------------------------------------------------------------

.PHONY:
	bench clean clean-fully
