#Alpine build container
FROM alpine:latest
RUN apk add make musl-dev gcc fcgi-dev xxhash-dev xxhash-static
WORKDIR /wrksrc
CMD make clean && make CC="x86_64-alpine-linux-musl-cc -static"
