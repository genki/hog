FROM alpine:edge AS builder
RUN apk --no-cache add make g++ musl-dev cutter lz4 git autoconf automake \
  jemalloc zeromq libevent msgpack-c-dev ca-certificates cmake libtool icu-dev
RUN update-ca-certificates
WORKDIR /mnt
COPY Makefile config.h /mnt/
COPY src ./src
COPY lib ./lib
COPY include ./include
RUN make hog

FROM alpine:edge
COPY --from=builder /mnt/hog /usr/bin
RUN mkdir -p /var/lib/hog
WORKDIR /var/lib/hog
EXPOSE 18618
ENTRYPOINT ["hog"]
