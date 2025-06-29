## While this script is running, you will see a file `running_silo.flag` created in the current directory.
## It will contain the PID of the running Silo API server.
## To kill the Silo API server, you can run `make clean`.

SILO_EXECUTABLE=./build/Debug/silo
RUNNING_SILO_FLAG=running_silo.flag

${SILO_EXECUTABLE}:
	python3 build_with_conan.py

output: ${SILO_EXECUTABLE}
	${SILO_EXECUTABLE} preprocessing --database-config database_config.yaml --preprocessing-config testBaseData/test_preprocessing_config.yaml


${RUNNING_SILO_FLAG}: ${SILO_EXECUTABLE} output
	@{ \
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
	(cd endToEndTests && SILO_URL=localhost:8081 npm run test)

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
	full-clean clean clean-api e2e
