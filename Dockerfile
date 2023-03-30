FROM alpine:3.17.0 AS builder

RUN apk update && apk add --no-cache py3-pip \
    build-base=0.5-r3 \
    cmake=3.24.4-r0 \
    linux-headers=5.19.5-r0 \
    boost-build=1.79.0-r0 \
    libtbb=2021.7.0-r0

RUN pip install conan==2.0.2

WORKDIR /src
COPY conanfile.py conanprofile.docker .
RUN mv conanprofile.docker conanprofile

RUN --mount=type=cache,target=/root/.conan2 \
    conan install . --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build

COPY . .

RUN  \
    --mount=type=cache,target=/root/.conan2 \
    --mount=type=cache,target=build \
    ash ./build_with_conan.sh release \
    && cp build/silo_test . \
    && cp build/siloApi .


FROM alpine:3.17.0 AS server

RUN apk update && apk add libtbb=2021.7.0-r0 curl jq

WORKDIR /app
COPY testBaseData .
COPY --from=builder /src/siloApi .

# call /info, extract "seqeunceCount" from the JSON and assert that the value is not 0. If any of those fails, "exit 1".
HEALTHCHECK --start-period=20s CMD curl --fail --silent localhost:8081/info | jq .sequenceCount | xargs test 0 -ne || exit 1

EXPOSE 8081
ENV SPDLOG_LEVEL="off,file_logger=debug"
CMD ["./siloApi", "--api"]