FROM alpine:edge
RUN apk --no-cache add make g++ musl-dev cutter lz4 git autoconf automake \
  jemalloc zeromq libevent msgpack-c-dev ca-certificates cmake libtool icu-dev
RUN update-ca-certificates
WORKDIR /mnt
