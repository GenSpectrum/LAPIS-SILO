
# Required env vars:

# The name of a subfolder of `~/silo-benchmark-datasets/` where the
# dataset is stored
ifeq ($(DATASET),)
$(error "DATASET must be a subfolder name, got '$(DATASET)'")
endif

# Should sorted input be used?
ifeq ($(SORTED),0)
INPUT_FILE = $(BENCHMARK_DATASET_DIR)/input_file.ndjson.zst
OUTPUT_DIR = $(DATASET)-output
else ifeq ($(SORTED),1)
# OK to write back into the $(BENCHMARK_DATASET_DIR)?
INPUT_FILE = $(BENCHMARK_DATASET_DIR)/sorted_input_file.ndjson.zst
OUTPUT_DIR = $(DATASET)-sorted_output
else
$(error "SORTED must be either 0 or 1, got '$(SORTED)'")
endif
export OUTPUT_DIR

# Should API query order be randomized?
ifeq ($(RANDOMIZED),0)
API_QUERY_OPTS=
else ifeq ($(RANDOMIZED),1)
API_QUERY_OPTS=--randomize
else
$(error "RANDOMIZED must be either 0 or 1, got '$(RANDOMIZED)'")
endif

# Passed to api-query
ifeq ($(CONCURRENCY),)
$(error "CONCURRENCY must be a natural number, missing")
endif

# Passed to api-query
ifeq ($(REPEAT),)
$(error "REPEAT must be a natural number, missing")
endif


# ---
# Added by evobench-run, and we rely on it
ifeq ($(COMMIT_ID),)
$(error "COMMIT_ID must be set")
endif


# ---- General -----------------------------------------------------------------

BENCHMARK_DATASET_DIR=$(HOME)/silo-benchmark-datasets/$(DATASET)

SILO_PREPROCESSING_CONFIG=$(BENCHMARK_DATASET_DIR)/preprocessing_config.yaml
export SILO_PREPROCESSING_CONFIG
SILO_INPUT_DIRECTORY=$(BENCHMARK_DATASET_DIR)
export SILO_INPUT_DIRECTORY

all: bench

# ---- Get dependencies --------------------------------------------------------

API_QUERY_VERSION=b5cf48baa00b8a1b95feaa28ade374b0b68de7e4

API_QUERY_DIR=api-query
API_QUERY_CLONE=$(API_QUERY_DIR)/Cargo.toml
API_QUERY_CHECKOUT_STAMP_DIR=$(API_QUERY_DIR)/.git/.checkout
API_QUERY_CHECKOUT=$(API_QUERY_CHECKOUT_STAMP_DIR)/$(API_QUERY_VERSION)
API_QUERY=$(API_QUERY_DIR)/target/release/api-query

$(API_QUERY_CLONE):
	git clone https://github.com/GenSpectrum/api-query $(API_QUERY_DIR)

$(API_QUERY_CHECKOUT): $(API_QUERY_CLONE)
	rm -rf $(API_QUERY_CHECKOUT_STAMP_DIR) # remove previous version stamp
	( cd $(API_QUERY_DIR) && git remote update && git checkout -b local_hidden_$(API_QUERY_VERSION) $(API_QUERY_VERSION) )
	mkdir -p $(API_QUERY_CHECKOUT_STAMP_DIR)
	touch $(API_QUERY_CHECKOUT)

$(API_QUERY): $(API_QUERY_CHECKOUT)
	cd $(API_QUERY_DIR) && cargo build --release


WISEPULSE_DIR=WisePulse
WISEPULSE_CHECKOUT=$(WISEPULSE_DIR)/Cargo.toml
WISEPULSE_BIN=$(WISEPULSE_DIR)/target/release
# Build the other binaries at the same time, too
WISEPULSE=$(WISEPULSE_BIN)/split_into_sorted_chunks

$(WISEPULSE_CHECKOUT):
	git clone https://github.com/cbg-ethz/WisePulse $(WISEPULSE_DIR)
	cd $(WISEPULSE_DIR) && git checkout -b sort-by-metadata-hack origin/sort-by-metadata-hack

$(WISEPULSE): $(WISEPULSE_CHECKOUT)
	cd $(WISEPULSE_DIR) && cargo build --release


# just for manual call comfort
dependencies: $(API_QUERY) $(WISEPULSE)


# ---- Sorting input -----------------------------------------------------------

$(BENCHMARK_DATASET_DIR)/sorted_input_file.ndjson.zst: $(BENCHMARK_DATASET_DIR)/input_file.ndjson.zst
	make $(WISEPULSE)
	rm -rf sorted_chunks merger_tmp 
	zstdcat $(BENCHMARK_DATASET_DIR)/input_file.ndjson.zst \
	    | $(WISEPULSE_BIN)/split_into_sorted_chunks --output-path sorted_chunks --chunk-size 100000 --sort-field date \
	    | $(WISEPULSE_BIN)/merge_sorted_chunks --tmp-directory merger_tmp --sort-field date \
	    | zstd > $(BENCHMARK_DATASET_DIR)/sorted_input_file.ndjson.zst.tmp
	mv $(BENCHMARK_DATASET_DIR)/sorted_input_file.ndjson.zst.tmp $(BENCHMARK_DATASET_DIR)/sorted_input_file.ndjson.zst

# ---- Running silo -----------------------------------------------------------

SILO=../build/Release/silo_$(COMMIT_ID)
export SILO
API_OPTIONS=--api-threads-for-http-connections 16
export API_OPTIONS
PREPROCESSING_STAMP = $(OUTPUT_DIR)/.done

$(SILO):
	bin/build-silo

$(PREPROCESSING_STAMP): $(SILO) $(INPUT_FILE)
	$(SILO) preprocessing --ndjson-input-filename $(INPUT_FILE) --output-directory $(OUTPUT_DIR)
	mv -f $(EVOBENCH_LOG) $(EVOBENCH_LOG)-preprocessing.log
	touch $(PREPROCESSING_STAMP)

.silo.pid: $(PREPROCESSING_STAMP)
	rm -f .silo.stopped
	bin/start-silo

.silo.stopped:
	bin/stop-silo
	touch .silo.stopped

bench: $(BENCHMARK_DATASET_DIR)/silo_queries.ndjson $(API_QUERY) .silo.stopped
	@echo "running benchmark"
	make .silo.pid
	$(API_QUERY) iter $(API_QUERY_OPTS) --repeat $(REPEAT) $(BENCHMARK_DATASET_DIR)/silo_queries.ndjson --drop --concurrency $(CONCURRENCY)
	make .silo.stopped

clean:
	rm -rf *-output/ *-sorted_output/ logs sorted_chunks merger_tmp

clean-fully: clean
	rm -rf ../build

# ----------------------------------------------------------------------------

.PHONY:
	bench clean clean-fully $(SILO)
