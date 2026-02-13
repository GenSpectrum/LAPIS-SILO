## While this script is running, you will see a file `running_silo.flag` created in the current directory.
## It will contain the PID of the running silo API server.
## If something fails during execution, it might be necessary to kill the daemon using `make clean-api`.
SILO_DEBUG_EXECUTABLE=./build/Debug/silo
SILO_RELEASE_EXECUTABLE=./build/Release/silo
RUNNING_SILO_FLAG=running_silo.flag
DEPENDENCIES_FLAG=dependencies
CLANG_FORMAT=$(shell command -v clang-format-19 2>/dev/null || command -v clang-format 2>/dev/null || echo clang-format)

ci: format all-tests

conanprofile:
	conan profile detect || true
	conan profile show --context build > conanprofile

${DEPENDENCIES_FLAG}: conanfile.py conanprofile
	conan install . --update --build=missing --profile ./conanprofile --profile:build ./conanprofile \
	  --settings '&:build_type=Debug' --settings 'arrow/*:compiler.cppstd=20' \
	  --output-folder=build/Debug/generators && \
	conan install . --update --build=missing --profile ./conanprofile --profile:build ./conanprofile \
	  --settings 'arrow/*:compiler.cppstd=20' \
	  --output-folder=build/Release/generators && \
	touch ${DEPENDENCIES_FLAG}

build/Debug/build.ninja: ${DEPENDENCIES_FLAG}
	cmake -B build/Debug -D CMAKE_BUILD_TYPE=Debug

build/Release/build.ninja: ${DEPENDENCIES_FLAG}
	cmake -B build/Release -D CMAKE_BUILD_TYPE=Release

${SILO_DEBUG_EXECUTABLE}: build/Debug/build.ninja $(shell find src -type f)
	cmake --build build/Debug --parallel 16

${SILO_RELEASE_EXECUTABLE}: build/Release/build.ninja $(shell find src -type f)
	cmake --build build/Release --parallel 16

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

e2e: ${RUNNING_SILO_FLAG}
	trap 'make clean-api' EXIT; \
	(cd endToEndTests && SILO_URL=localhost:8093 npm run test)

test: ${SILO_DEBUG_EXECUTABLE}
	build/Debug/silo_test --gtest_filter='*' --gtest_color=no

python-tests: ${DEPENDENCIES_FLAG}
	uv venv --allow-existing .venv
	uv pip install -q setuptools wheel 'Cython>=3.0.0'
	uv pip install -q --no-build-isolation ".[test]"
	.venv/bin/pytest python/tests -v

build-wheel: conanprofile
	python3 ./build_with_conan.py --release
	rm -rf dist
	@if [ "$$(uname)" = "Darwin" ]; then \
		_PYTHON_HOST_PLATFORM="macosx-15.0-$$(uname -m)" uv build --wheel; \
	else \
		uv build --wheel; \
	fi
	mkdir -p wheelhouse
	@if [ "$$(uname)" = "Darwin" ]; then \
		uv tool run --from delocate delocate-wheel -w wheelhouse/ dist/*.whl; \
	else \
		uv tool run auditwheel repair dist/*.whl -w wheelhouse/; \
	fi

all-tests: test e2e python-tests

endToEndTests/node_modules: endToEndTests/package-lock.json
	cd endToEndTests && npm ci

format-cpp:
	find src -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs $(CLANG_FORMAT) -i

format-node: endToEndTests/node_modules
	cd endToEndTests && npm run format
	cd endToEndTests && npx prettier --write "../.github/workflows/*.yml"

format: format-cpp format-node

check-format-cpp:
	find src -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs $(CLANG_FORMAT) --dry-run --Werror

check-format-node: endToEndTests/node_modules
	cd endToEndTests && npm run check-format
	cd endToEndTests && npx prettier --check "../.github/workflows/*.yml"

check-format: check-format-cpp check-format-node

clean-api:
	@if [ -f ${RUNNING_SILO_FLAG} ]; then \
		kill $$(cat ${RUNNING_SILO_FLAG}) || true; \
		rm -f ${RUNNING_SILO_FLAG}; \
	fi

clean: clean-api
	rm -rf output logs ${DEPENDENCIES_FLAG}

full-clean: clean
	rm -rf build

.PHONY:
	full-clean clean clean-api e2e format format-cpp format-node check-format check-format-cpp check-format-node all test all-tests ci python-tests build-wheel
