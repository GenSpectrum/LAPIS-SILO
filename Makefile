# Note: you can set the COLOR environment variable to 1 to get the log
# files produced with color codes.

export PATH := bin:$(PATH)

all: build/siloApi build/silo_test

clean:
	find build -name "*.o" -print0 | xargs -0 rm -f
	rm -f build/siloApi build/silo_test

# Have separate targets for the binaries, but share a single build
# (faster to build them both?); use `run-cached` to only actually run one
# build, though.

build/siloApi: $(shell bin/cplusplus-source-files)
	run-cached build/.exit-code build $@

build/silo_test: $(shell bin/cplusplus-source-files)
	run-cached build/.exit-code build $@


# Tests produce log files if successful (if not successful, the log
# file can be found with .tmp appended, but is also printed to stdout).

build/unit-tests.log: build/silo_test
	run-with-log $@ build/silo_test

build/preprocessing-tsv.log: \
        build/siloApi \
        $(shell bin/preprocessing-input-files-in testBaseData/exampleDataset)
	run-with-log $@ preprocessing-in testBaseData/exampleDataset

build/preprocessing-ndjson.log: \
        build/siloApi \
        $(shell bin/preprocessing-input-files-in testBaseData/exampleDatasetAsNdjson)
	run-with-log $@ preprocessing-in testBaseData/exampleDatasetAsNdjson

build/tsv-tests.log: build/siloApi build/preprocessing-tsv.log $(shell bin/test-query-files)
	run-with-log $@ runtests-e2e testBaseData/exampleDataset 7001

build/ndjson-tests.log: build/siloApi build/preprocessing-ndjson.log $(shell bin/test-query-files)
	run-with-log $@ runtests-e2e testBaseData/exampleDatasetAsNdjson 7002

test: build/unit-tests.log build/tsv-tests.log build/ndjson-tests.log


# Manually run the api so that it can be queried interactively.

runapi-tsv: build/siloApi build/preprocessing-tsv.log
	runapi-in testBaseData/exampleDataset 8081

runapi-ndjson: build/siloApi build/preprocessing-tsv.log
	runapi-in testBaseData/exampleDatasetAsNdjson 8081


.PHONY: all clean test runapi-tsv runapi-ndjson
