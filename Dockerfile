FROM alpine:3.17.0 AS dep_builder

RUN apk update && apk add --no-cache py3-pip \
    build-base=0.5-r3 \
    cmake=3.24.4-r0 \
    linux-headers=5.19.5-r0 \
    boost-build=1.79.0-r0 \
    libtbb=2021.7.0-r0

RUN pip install conan==2.0.8

WORKDIR /src
COPY conanfile.py conanprofile.docker ./
RUN mv conanprofile.docker conanprofile

RUN --mount=type=cache,target=/root/.conan2 \
    conan install . --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build \
    && cp -R /root/.conan2 /root/.conan2_persisted && cp -R build build_persisted

# Restore the cached directories as the cache mount deletes them.
# We need this because cache mounts are not cached in GitHub Actions
# (see https://github.com/docker/build-push-action/issues/716)
RUN cp -R /root/.conan2_persisted /root/.conan2 && cp -R build_persisted build

FROM dep_builder AS builder

COPY . ./

RUN  \
    python ./build_with_conan.py --release --parallel 4\
    && cp build/silo_test . \
    && cp build/siloApi .


FROM alpine:3.17.0 AS server

WORKDIR /app
COPY docker_default_preprocessing_config.yaml ./default_preprocessing_config.yaml
COPY docker_runtime_config.yaml ./runtime_config.yaml
COPY --from=builder /src/siloApi ./

RUN apk update && apk add libtbb=2021.7.0-r0 curl jq

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081

ENTRYPOINT ["./siloApi"]
