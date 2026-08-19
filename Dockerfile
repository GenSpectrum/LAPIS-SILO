ARG DEPENDENCY_IMAGE=ghcr.io/genspectrum/lapis-silo-dependencies:latest

FROM $DEPENDENCY_IMAGE AS builder

COPY . ./

RUN  \
    export CMAKE_BUILD_PARALLEL_LEVEL=4; \
    make build/Release/rhydb_test \
    && make build/Release/rhydb_app_test \
    && make build/Release/rhydb \
    && cp build/Release/rhydb_test . \
    && cp build/Release/rhydb_app_test . \
    && cp build/Release/rhydb .


FROM ubuntu:26.04 AS server

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./default_runtime_config.yaml
COPY --from=builder /src/rhydb ./

# Deprecation compatibility: the binary was renamed silo -> rhydb. Keep a
# `silo` shim in the image during the deprecation period so consumers that
# invoke the binary by its old name (e.g. `--entrypoint ./silo` or scripts
# that exec /app/silo) keep working. Remove once the deprecation period ends.
RUN ln -s rhydb /app/silo

RUN apt update && apt dist-upgrade -y \
    &&  apt install -y curl jq

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./rhydb"]

ENV SILO_PREPROCESSING_CONFIG="/app/preprocessing_config.yaml"
ENV SILO_DEFAULT_PREPROCESSING_CONFIG="/app/default_preprocessing_config.yaml"
ENV SILO_DEFAULT_RUNTIME_CONFIG="/app/default_runtime_config.yaml"

LABEL org.opencontainers.image.source="https://github.com/GenSpectrum/LAPIS-SILO"
LABEL org.opencontainers.image.description="High-performance analytical database for sequence alignment data"
