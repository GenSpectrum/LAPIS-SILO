ARG DEPENDENCY_IMAGE=ghcr.io/genspectrum/lapis-silo-dependencies:latest

FROM $DEPENDENCY_IMAGE AS builder

COPY . ./

RUN  \
    python ./build_with_conan.py --release --parallel 4\
    && cp build/silo_test . \
    && cp build/siloApi .


FROM alpine:3.20 AS server

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./runtime_config.yaml
COPY --from=builder /src/siloApi ./

RUN apk update && apk add onetbb=2021.12.0-r0 curl jq

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./siloApi"]

LABEL org.opencontainers.image.source="https://github.com/GenSpectrum/LAPIS-SILO"
LABEL org.opencontainers.image.description="Sequence Indexing engine for Large Order of genomic data"
