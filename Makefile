default: image

build: build/ok
build/ok: build/Dockerfile
	docker build -t hog-build build
	touch build/ok
build/hog:
	docker run --rm -v $(CURDIR):/mnt hog-build make -C build hog

image: image.ok
image.ok: Dockerfile build/ok build/hog
	docker build -t s21g/hog:longa .
	touch image.ok

clean:
	make -C build clean

.PHONY: clean
