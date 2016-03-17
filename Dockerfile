FROM ubuntu:trusty
MAINTAINER Genki Takiuchi <genki@s21g.com>

# groonga
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get install -y software-properties-common
RUN add-apt-repository -y universe
RUN add-apt-repository -y ppa:groonga/ppa
RUN apt-get update
RUN apt-get install -y groonga libgroonga-dev
RUN echo "vm.max_map_count = 524288" >> /etc/sysctl.conf
RUN sysctl -w vm.max_map_count=524288

# hog
RUN apt-get install -y cmake g++ make
WORKDIR /usr/src
RUN mkdir -p hog
WORKDIR hog
ADD src ./src
ADD CMakeLists.txt ./
ADD cmake ./cmake
RUN cmake .
RUN make all install

EXPOSE 18618
ENTRYPOINT ["hog"]
