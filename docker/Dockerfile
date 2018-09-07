FROM gliderlabs/alpine:latest

MAINTAINER tyfkda

ENV IMAGE_NAME=haribote-dev

RUN apk update && \
    apk --no-cache add \
        build-base \
        ruby \
        xorriso \
    && echo 'DONE!'

CMD ["/bin/ash"]
