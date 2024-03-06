FROM alpine:3.18 AS dep_builder

RUN apk update && apk add --no-cache py3-pip \
    build-base=0.5-r3 \
    cmake=3.26.5-r0 \
    bash=5.2.15-r5 \
    linux-headers=6.3-r0 \
    boost-build=1.82.0-r0 \
    libtbb=2021.9.0-r0

RUN pip install conan==2.0.17

WORKDIR /src
COPY conanfile.py conanprofile.docker ./
RUN mv conanprofile.docker conanprofile

RUN conan install . --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build

FROM dep_builder AS builder

COPY . ./

RUN  \
    python ./build_with_conan.py --release --parallel 4\
    && cp build/silo_test . \
    && cp build/siloApi .


FROM alpine:3.18 AS server

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./runtime_config.yaml
COPY --from=builder /src/siloApi ./

RUN apk update && apk add libtbb=2021.9.0-r0 curl jq

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./siloApi"]
