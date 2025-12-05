## While this script is running, you will see a file `running_silo.flag` created in the current directory.
## It will contain the PID of the running silo API server.
## If something fails during execution, it might be necessary to kill the daemon using `make clean-api`.
SILO_EXECUTABLE=./build/Debug/silo
RUNNING_SILO_FLAG=running_silo.flag
DEPENDENCIES_FLAG=dependencies

ci: format all-tests

conanprofile:
	conan profile detect && conan profile show --context build > conanprofile

${DEPENDENCIES_FLAG}: conanfile.py conanprofile
	conan install . --update --build=missing --profile ./conanprofile --profile:build ./conanprofile \
	  --settings '&:build_type=Debug' --output-folder=build/Debug/generators && \
	touch ${DEPENDENCIES_FLAG}

build/Debug/build.ninja: ${DEPENDENCIES_FLAG}
	cmake -B build/Debug -D CMAKE_BUILD_TYPE=Debug

${SILO_EXECUTABLE}: build/Debug/build.ninja $(shell find src -type f)
	cmake --build build/Debug --parallel 16

output: ${SILO_EXECUTABLE}
	export SPDLOG_LEVEL=debug; \
	${SILO_EXECUTABLE} preprocessing --database-config database_config.yaml --preprocessing-config testBaseData/test_preprocessing_config.yaml


${RUNNING_SILO_FLAG}: ${SILO_EXECUTABLE} output
	@{ \
		if lsof -i :8093 > /dev/null 2>&1; then \
			echo "Error: Port 8093 is already in use. Another SILO instance might already be running."; \
			exit 1; \
		fi; \
		${SILO_EXECUTABLE} api --api-port 8093 & \
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

test: ${SILO_EXECUTABLE}
	build/Debug/silo_test --gtest_filter='*' --gtest_color=no

python-tests:
	pip install -q pytest
	pip install -q .
	pytest python/tests -v

all-tests: test e2e python-tests

endToEndTests/node_modules: endToEndTests/package-lock.json
	cd endToEndTests && npm ci

format: endToEndTests/node_modules
	find src -iname '*.h' -o -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
	cd endToEndTests && npm run format
	cd endToEndTests && npx prettier --write "../.github/workflows/*.yml"

check-format: endToEndTests/node_modules
	cd endToEndTests && npm run check-format
	cd endToEndTests && npx prettier --check "../.github/workflows/*.yml"

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
	full-clean clean clean-api e2e format check-format all test all-tests ci python-tests
