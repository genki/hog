FROM alpine:edge

ENV VER 6.1.1
RUN apk --no-cache add make g++ musl-dev wget \
  jemalloc zeromq libevent msgpack-c-dev ca-certificates
WORKDIR /usr/local/src
RUN update-ca-certificates
RUN wget http://packages.groonga.org/source/groonga/groonga-$VER.tar.gz
RUN tar xzf groonga-$VER.tar.gz
WORKDIR groonga-$VER

ENV CFLAGS -g -O2 -fPIE -fstack-protector-strong -Wformat -Werror=format-security
ENV LDFLAGS -Wl,-Bsymbolic-functions -fPIE -pie -Wl,-z,relro -Wl,-z,now
ENV CPPFLAGS -Wdate-time -D_FORTIFY_SOURCE=2
ENV CXXFLAGS -g -O2 -fPIE -fstack-protector-strong -Wformat -Werror=format-security

RUN ./configure --prefix=/usr \
  --disable-maintainer-mode --disable-dependency-tracking \
  --disable-groonga-httpd
RUN make
RUN make install
WORKDIR ..
RUN rm -rf *

# for hog
RUN apk --no-cache add cmake
WORKDIR /usr/src
RUN mkdir -p hog
WORKDIR hog
ADD src ./src
ADD CMakeLists.txt ./
ADD cmake ./cmake
RUN cmake .
RUN make all install
WORKDIR ..
RUN rm -rf hog

EXPOSE 18618
ENTRYPOINT ["hog"]
