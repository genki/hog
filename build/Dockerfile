FROM alpine:latest
RUN apk --no-cache add make g++ musl-dev cutter \
  libevent msgpack-c-dev ca-certificates libtool icu-dev
RUN update-ca-certificates
COPY --from=groonga/groonga:latest-alpine /usr/include/groonga /usr/include
COPY --from=groonga/groonga:latest-alpine /usr/lib/libgroonga* /usr/lib/
WORKDIR /mnt
