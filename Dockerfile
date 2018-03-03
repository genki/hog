FROM alpine:edge
COPY ./build/hog /usr/bin
RUN mkdir -p /var/lib/hog
WORKDIR /var/lib/hog
EXPOSE 18618
ENTRYPOINT ["hog"]
