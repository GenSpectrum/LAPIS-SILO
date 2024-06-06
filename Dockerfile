ARG DEPENDENCY_IMAGE=ghcr.io/genspectrum/lapis-silo-dependencies:latest

FROM $DEPENDENCY_IMAGE AS builder

COPY . ./

RUN python3 ./build_with_conan.py --release --parallel 4 \
    && cp build/silo_test . \
    && cp build/siloApi .


FROM ubuntu:24.04 AS server

LABEL org.opencontainers.image.source="https://github.com/GenSpectrum/LAPIS-SILO"
LABEL org.opencontainers.image.description="Sequence Indexing engine for Large Order of genomic data"

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    curl \
    jq \
    libtbb12 \
    && apt-get clean \
    && apt-get autoremove --purge -y

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./runtime_config.yaml
COPY --from=builder /src/siloApi ./


# call /info, extract "sequenceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
#TODO: This health check wrongly assumes that no sequences means there is a problem.
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./siloApi"]
