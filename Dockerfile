ARG DEPENDENCY_IMAGE=ghcr.io/genspectrum/lapis-silo-dependencies:latest

FROM $DEPENDENCY_IMAGE AS builder

COPY . ./

RUN  \
    python3 ./build_with_conan.py --release --parallel 4\
    && cp build/Release/silo_test . \
    && cp build/Release/silo .


FROM ubuntu:24.04 AS server

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./default_runtime_config.yaml
COPY --from=builder /src/silo ./

RUN apt update && apt dist-upgrade -y \
    &&  apt install -y libtbb12 curl jq

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./silo"]

ENV SILO_PREPROCESSING_CONFIG="/app/preprocessing_config.yaml"
ENV SILO_DEFAULT_PREPROCESSING_CONFIG="/app/default_preprocessing_config.yaml"
ENV SILO_DEFAULT_RUNTIME_CONFIG="/app/default_runtime_config.yaml"

LABEL org.opencontainers.image.source="https://github.com/GenSpectrum/LAPIS-SILO"
LABEL org.opencontainers.image.description="Sequence Indexing engine for Large Order of genomic data"
