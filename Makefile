# Note: you can set the COLOR environment variable to 1 to get the log
# files produced with color codes.

export PATH := bin:$(PATH)

all: build/Debug/siloApi build/Debug/silo_test

clean:
	find build -name "*.o" -print0 | xargs -0 rm -f
	rm -f build/Debug/siloApi build/Debug/silo_test

# Have separate targets for the binaries, but share a single build
# (faster to build them both?); use `run-cached` to only actually run one
# build, though.

build/Debug/siloApi: $(shell bin/cplusplus-source-files)
	run-cached build/Debug/.exit-code build $@

build/Debug/silo_test: $(shell bin/cplusplus-source-files)
	run-cached build/Debug/.exit-code build $@


# Tests produce log files if successful (if not successful, the log
# file can be found with .tmp appended, but is also printed to stdout).

build/Debug/unit-tests.log: build/Debug/silo_test
	run-with-log $@ build/Debug/silo_test

build/Debug/preprocessing.log: \
        build/Debug/siloApi \
        $(shell bin/preprocessing-input-files-in testBaseData/exampleDataset)
	run-with-log $@ preprocessing-in testBaseData/exampleDataset

build/Debug/integration-tests.log: build/Debug/siloApi build/Debug/preprocessing.log $(shell bin/test-query-files)
	run-with-log $@ runtests-e2e testBaseData/exampleDataset 7002

test: build/Debug/unit-tests.log build/Debug/integration-tests.log


# Manually run the api so that it can be queried interactively.

runapi: build/Debug/siloApi build/Debug/preprocessing.log
	runapi-in testBaseData/exampleDataset 8081


.PHONY: all clean test runapi
