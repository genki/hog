FROM alpine:edge
RUN apk --no-cache add make g++ musl-dev cutter lz4 \
  jemalloc libevent msgpack-c-dev ca-certificates libtool icu-dev
RUN update-ca-certificates
COPY --from=groonga/groonga:8.0.0 /usr/include/groonga /usr/include
COPY --from=groonga/groonga:8.0.0 /usr/lib/libgroonga* /usr/lib/
WORKDIR /mnt
