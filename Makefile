default: image

lib_c_SOURCES := $(shell find lib -type f -name "*.c")
lib_cc_SOURCES := $(shell find lib -type f -name "*.cpp")
hog_SOURCES := $(shell find src -type f -name "*.c")
SOURCES := $(lib_c_SOURCES) $(lib_cc_SOURCES) $(hog_SOURCES)
c_OBJECTS := $(lib_c_SOURCES:.c=.o) $(hog_SOURCES:.c=.o)
cc_OBJECTS := $(lib_cc_SOURCES:.cpp=.o)
OBJECTS := $(c_OBJECTS) $(cc_OBJECTS)
CC := gcc
CXX := g++

CFLAGS := -g -O2 -fPIE -fstack-protector-strong \
	-Wformat -Werror=format-security \
	-I include \
	-DHAVE_CONFIG_H=1 \
	-DGRN_PLUGINS_DIR=\"/usr/lib/groonga/plugins\"
LDFLAGS := -Wl,-Bsymbolic-functions -fPIE -pie -Wl,-z,relro -Wl,-z,now -static

$(c_OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
$(cc_OBJECTS): %.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

hog: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

build: build/ok
build/ok: build/Dockerfile
	docker build -t hog-build build
	touch build/ok
build/hog: $(SOURCES)
	docker run --rm -v $(CURDIR):/mnt hog-build make hog
	mv ./hog build/hog

image: image.ok
image.ok: Dockerfile build/ok build/hog
	docker build -t s21g/hog:longa .
	touch image.ok

run: build/ok
	docker run -it --rm -p 18618:18618 -v $(CURDIR):/mnt \
		--entrypoint build/hog hog-build -l /dev/stdout -L debug tmp/db/test
clean:
	rm -f $(OBJECTS) build/hog

.PHONY: clean run
