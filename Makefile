## While this script is running, you will see a file `running_silo.flag` created in the current directory.
## It will contain the PID of the running silo API server.
## If something fails during execution, it might be necessary to kill the daemon using `make clean-api`.
SILO_DEBUG_EXECUTABLE=./build/Debug/silo
SILO_DEBUG_TEST_EXECUTABLE=./build/Debug/silo_test
SILO_RELEASE_EXECUTABLE=./build/Release/silo
SILO_RELEASE_TEST_EXECUTABLE=./build/Release/silo_test
RUNNING_SILO_FLAG=running_silo.flag
DEPENDENCIES_FLAG=dependencies
CLANG_FORMAT=$(shell command -v clang-format-19 2>/dev/null || command -v clang-format 2>/dev/null || echo clang-format)
CMAKE_BUILD_PARALLEL_LEVEL ?= 16

.PHONY: ci
ci: format all-tests

.PHONY: conanprofile
conanprofile:
	buildScripts/create-conanprofile

${DEPENDENCIES_FLAG}: conanfile.py conanprofile
	buildScripts/install-dependencies
	touch ${DEPENDENCIES_FLAG}

SRC_FILE_LIST=.src_file_list
$(SRC_FILE_LIST): FORCE
	@find src -type f | sort > $@.tmp
	@cmp -s $@ $@.tmp && rm $@.tmp || mv $@.tmp $@
.PHONY: FORCE

build/Debug/build.ninja: ${DEPENDENCIES_FLAG} $(SRC_FILE_LIST)
	cmake -G Ninja -B build/Debug -D CMAKE_BUILD_TYPE=Debug

build/Release/build.ninja: ${DEPENDENCIES_FLAG} $(SRC_FILE_LIST)
	cmake -G Ninja -B build/Release -D CMAKE_BUILD_TYPE=Release

${SILO_DEBUG_EXECUTABLE}: build/Debug/build.ninja $(shell find src -type f)
	cmake --build build/Debug --parallel $(CMAKE_BUILD_PARALLEL_LEVEL) --target silo

${SILO_DEBUG_TEST_EXECUTABLE}: build/Debug/build.ninja $(shell find src -type f)
	cmake --build build/Debug --parallel $(CMAKE_BUILD_PARALLEL_LEVEL) --target silo_test

${SILO_RELEASE_EXECUTABLE}: build/Release/build.ninja $(shell find src -type f)
	cmake --build build/Release --parallel $(CMAKE_BUILD_PARALLEL_LEVEL) --target silo

${SILO_RELEASE_TEST_EXECUTABLE}: build/Release/build.ninja $(shell find src -type f)
	cmake --build build/Release --parallel $(CMAKE_BUILD_PARALLEL_LEVEL) --target silo_test

.PHONY: output
output: ${SILO_DEBUG_EXECUTABLE}
	export SPDLOG_LEVEL=debug; \
	${SILO_DEBUG_EXECUTABLE} preprocessing --database-config database_config.yaml --preprocessing-config testBaseData/test_preprocessing_config.yaml

${RUNNING_SILO_FLAG}: ${SILO_DEBUG_EXECUTABLE} output
	@{ \
		if lsof -i :8093 > /dev/null 2>&1; then \
			echo "Error: Port 8093 is already in use. Another SILO instance might already be running."; \
			exit 1; \
		fi; \
		${SILO_DEBUG_EXECUTABLE} api --api-port 8093 & \
		pid=$$!; \
		echo "Waiting for silo (PID $$pid) to be ready..."; \
		until curl -s -o /dev/null -w "%{http_code}" http://localhost:8093/info | grep -q "200"; do \
			sleep 0.2; \
		done; \
		echo $$pid > ${RUNNING_SILO_FLAG}; \
		echo "Silo is running with PID $$pid"; \
	}

.PHONY: e2e
e2e: ${RUNNING_SILO_FLAG}
	trap 'make clean-api' EXIT; \
	(cd endToEndTests && SILO_URL=localhost:8093 npm run test)

.PHONY: test
test: ${SILO_DEBUG_TEST_EXECUTABLE}
	${SILO_DEBUG_TEST_EXECUTABLE} --gtest_filter='*' --gtest_color=no

.PHONY: python-tests
python-tests: ${DEPENDENCIES_FLAG}
	uv venv --allow-existing .venv
	uv pip install -q setuptools wheel 'Cython>=3.0.0'
	uv pip install -q --no-build-isolation ".[test]"
	.venv/bin/pytest python/tests -v


PYTHON_VERSIONS ?= 3.11 3.12 3.13 3.14

.PHONY: build-wheels
build-wheels: ${SILO_RELEASE_EXECUTABLE}
	mkdir -p wheelhouse
	@for pyversion in $(PYTHON_VERSIONS); do \
  		echo; \
		echo "--- Building wheel for Python $$pyversion ---"; \
		uv python install $$pyversion; \
		rm -rf dist; \
		if [ "$$(uname)" = "Darwin" ]; then \
			_PYTHON_HOST_PLATFORM="macosx-15.0-$$(uname -m)" uv build --wheel --python $$pyversion; \
			uv tool run --from delocate delocate-wheel -w wheelhouse/ dist/*.whl; \
		else \
			uv build --wheel --python $$pyversion; \
			uv tool run auditwheel repair dist/*.whl -w wheelhouse/; \
		fi; \
	done

.PHONY: all-tests
all-tests: test e2e python-tests

endToEndTests/node_modules: endToEndTests/package-lock.json
	cd endToEndTests && npm ci

.PHONY: format-cpp
format-cpp:
	find src -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs $(CLANG_FORMAT) -i

.PHONY: format-node
format-node: endToEndTests/node_modules
	cd endToEndTests && npm run format
	cd endToEndTests && npx prettier --write "../.github/workflows/*.yml"

.PHONY: format
format: format-cpp format-node

.PHONY: check-format-cpp
check-format-cpp:
	find src -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs $(CLANG_FORMAT) --dry-run --Werror

.PHONY: check-format-node
check-format-node: endToEndTests/node_modules
	cd endToEndTests && npm run check-format
	cd endToEndTests && npx prettier --check "../.github/workflows/*.yml"

.PHONY: check-format
check-format: check-format-cpp check-format-node

.PHONY: bump-serialization-version
bump-serialization-version:
	buildScripts/bump_serialization_version.sh

.PHONY: clean-api
clean-api:
	@if [ -f ${RUNNING_SILO_FLAG} ]; then \
		kill $$(cat ${RUNNING_SILO_FLAG}) || true; \
		rm -f ${RUNNING_SILO_FLAG}; \
	fi

.PHONY: clean
clean: clean-api
	rm -rf output logs ${DEPENDENCIES_FLAG} ${SRC_FILE_LIST}

.PHONY: full-clean
full-clean: clean
	rm -rf build
