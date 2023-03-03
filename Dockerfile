FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        cmake=3.24.3-r0 \
        boost1.80-dev=1.80.0-r3 \
        libtbb-dev=2021.7.0-r0 \
        readline-dev=8.2.0-r0 \
        xz-dev=5.2.9-r0 \
        rapidjson-dev=1.1.0-r4 \
        nlohmann-json=3.11.2-r0 \
        poco-dev=1.12.2-r1 \
        gtest-dev=1.12.1-r0

WORKDIR /src
COPY . .

RUN \
    --mount=type=cache,target=build/CMakeFiles \
    --mount=type=cache,target=build/.cmake \
     cmake -D CMAKE_BUILD_TYPE=Release -B build .
RUN \
    --mount=type=cache,target=build/CMakeFiles \
    --mount=type=cache,target=build/.cmake \
    cmake --build build

RUN cp build/siloWebApi . && cp testBaseData/* .
RUN cp build/silo_test .

CMD ["./siloWebApi", "--api"]