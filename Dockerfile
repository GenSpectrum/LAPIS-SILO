FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        cmake=3.24.3-r0 \
        boost1.80-dev=1.80.0-r3 \
        libtbb-dev=2021.7.0-r0 \
        readline-dev=8.2.0-r0 \
        xz-dev=5.2.9-r0 \
        rapidjson-dev=1.1.0-r4

WORKDIR /src
COPY . .

WORKDIR build


RUN cmake ..
RUN cmake --build .

WORKDIR /src
RUN cp build/silo . && cp testBaseData/* .

CMD ["./silo"]