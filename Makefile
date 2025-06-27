## While this script is running, you will see a file `running_silo.flag` created in the current directory.
## It will contain the PID of the running silo API server.
## If something fails during execution, it might be necessary to kill the daemon using `make clean-api`.
SILO_EXECUTABLE=./build/Debug/silo
RUNNING_SILO_FLAG=running_silo.flag

ci: format all-tests

${SILO_EXECUTABLE}: $(shell find src -type f)
	python3 build_with_conan.py

output: ${SILO_EXECUTABLE}
	${SILO_EXECUTABLE} preprocessing --database-config database_config.yaml --preprocessing-config testBaseData/test_preprocessing_config.yaml


${RUNNING_SILO_FLAG}: ${SILO_EXECUTABLE} output
	@{ \
		if lsof -i :8081 > /dev/null 2>&1; then \
			echo "Error: Port 8081 is already in use. Another SILO instance might already be running."; \
			exit 1; \
		fi; \
		${SILO_EXECUTABLE} api & \
		pid=$$!; \
		echo "Waiting for silo (PID $$pid) to be ready..."; \
		until curl -s -o /dev/null -w "%{http_code}" http://localhost:8081/info | grep -q "200"; do \
			sleep 0.2; \
		done; \
		echo $$pid > ${RUNNING_SILO_FLAG}; \
		echo "Silo is running with PID $$pid"; \
	}

e2e: ${RUNNING_SILO_FLAG}
	trap 'make clean-api' EXIT; \
	(cd endToEndTests && SILO_URL=localhost:8081 npm run test)

test: ${SILO_EXECUTABLE}
	build/Debug/silo_test --gtest_filter=* --gtest_color=no

all-tests: test e2e

format:
	find src -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i

clean-api:
	@if [ -f ${RUNNING_SILO_FLAG} ]; then \
    		kill $$(cat ${RUNNING_SILO_FLAG}) || true; \
    		rm -f ${RUNNING_SILO_FLAG}; \
    	fi

clean: clean-api
	rm -rf output logs

full-clean: clean
	rm -rf build

.PHONY:
	full-clean clean clean-api e2e format all test all-tests format ci
