image: image.ok
image.ok: Dockerfile
	docker build -t s21g/hog:longa .
	touch image.ok

clean:
	rm -f $(OBJECTS) hog

# for inside builder
lib_c_SOURCES := $(shell find lib -type f -name "*.c")
lib_cc_SOURCES := $(shell find lib -type f -name "*.cpp")
hog_SOURCES := $(shell find src -type f -name "*.c")
c_OBJECTS := $(lib_c_SOURCES:.c=.o) $(hog_SOURCES:.c=.o)
cc_OBJECTS := $(lib_cc_SOURCES:.cpp=.o)
OBJECTS := $(c_OBJECTS) $(cc_OBJECTS)

CFLAGS := -g -O2 -fPIE -fstack-protector-strong \
	-Wformat -Werror=format-security \
	-DHAVE_CONFIG_H=1 \
	-DGRN_PLUGINS_DIR=\"/usr/lib/groonga/plugins\"
LDFLAGS := -Wl,-Bsymbolic-functions -fPIE -pie -Wl,-z,relro -Wl,-z,now -static

$(c_OBJECTS): %.o: %.c
	gcc -c -I include $(CFLAGS) -o $@ $<
$(cc_OBJECTS): %.o: %.cpp
	g++ -c -I include $(CFLAGS) -o $@ $<

hog: $(OBJECTS)
	g++ $(LDFLAGS) $^ -o $@

.PHONY: clean
