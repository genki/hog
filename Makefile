default: image

VERSION := $(shell cat VERSION)
TAG := s21g/hog:$(VERSION)
lib_c_SOURCES := $(shell find lib -type f -name "*.c")
lib_cc_SOURCES := $(shell find lib -type f -name "*.cpp")
hog_SOURCES := $(shell find src -type f -name "*.c")
SOURCES := $(lib_c_SOURCES) $(lib_cc_SOURCES) $(hog_SOURCES)
c_OBJECTS := $(lib_c_SOURCES:.c=.o) $(hog_SOURCES:.c=.o)
cc_OBJECTS := $(lib_cc_SOURCES:.cpp=.o)
OBJECTS := $(c_OBJECTS) $(cc_OBJECTS)
CC := gcc
CXX := g++

CFLAGS := -g -O2 -I include -DHOG_VERSION=\"$(VERSION)\" \
	-fPIE -fstack-protector-strong -Wformat -Werror=format-security
LDFLAGS := -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now \
	-fPIE -pie -lgroonga

$(c_OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
$(cc_OBJECTS): %.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<
src/main.c: VERSION

hog: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

build: build/ok
build/ok: build/Dockerfile
	docker build --iidfile build/ok -t hog-build build
build/hog: $(SOURCES) VERSION
	docker run --rm -v $(CURDIR):/mnt hog-build make hog
	mv ./hog build/hog

image: ok
ok: Dockerfile build/ok build/hog
	DOCKER_BUILDKIT=1 docker build --iidfile ok -t $(TAG) .

.PHONY: clean run push tags

run: build/ok
	docker run -it --rm -p 18618:18618 -v $(CURDIR):/mnt \
		--entrypoint build/hog hog-build -l /dev/stdout -L debug tmp/db/test
clean:
	rm -f $(OBJECTS) build/hog
push: ok
	docker push $(TAG)
tags:
	ctags -R
