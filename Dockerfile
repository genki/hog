FROM alpine:3.8
RUN apk --no-cache add musl-dev cutter \
  jemalloc libevent msgpack-c-dev icu-dev
COPY --from=groonga/groonga:latest-alpine /usr/lib/libgroonga* /usr/lib/
COPY --chmod=0755 ./build/hog /usr/bin
RUN mkdir -p /var/lib/hog
WORKDIR /var/lib/hog
EXPOSE 18618
ENTRYPOINT ["hog"]
