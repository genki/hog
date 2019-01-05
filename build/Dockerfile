FROM alpine:3.8
RUN apk --no-cache add make g++ musl-dev cutter lz4 \
  jemalloc libevent msgpack-c-dev ca-certificates libtool icu-dev
RUN update-ca-certificates
COPY --from=groonga/groonga /usr/include/groonga /usr/include
COPY --from=groonga/groonga /usr/lib/libgroonga* /usr/lib/
WORKDIR /mnt
