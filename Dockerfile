FROM alpine:3.17.0 AS build

RUN apk update && apk add --no-cache py3-pip \
    build-base=0.5-r3 \
    cmake=3.24.3-r0 \
    linux-headers=5.19.5-r0 \
    boost-build=1.79.0-r0

RUN pip install conan==1.59.0

WORKDIR /src
COPY . .

RUN mv conanprofile.docker conanprofile
RUN --mount=type=cache,target=/root/.conan conan install . --build=missing --profile ./conanprofile -if=/build

RUN  \
    --mount=type=cache,target=/root/.conan \
    --mount=type=cache,target=build/CMakeFiles \
    --mount=type=cache,target=build/.cmake \
    ash ./build_with_conan.sh release


FROM alpine:3.17.0 AS server

RUN apk update && apk add libtbb=2021.7.0-r0

WORKDIR /app
COPY testBaseData .
COPY --from=build /src/build/siloApi .

CMD ["./siloApi", "--api"]