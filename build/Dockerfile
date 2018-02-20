FROM alpine:edge
RUN apk --no-cache add make g++ musl-dev cutter lz4 \
  jemalloc libevent msgpack-c-dev ca-certificates libtool icu-dev
RUN update-ca-certificates
WORKDIR /mnt
