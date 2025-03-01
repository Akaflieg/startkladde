FROM debian:bookworm

RUN mkdir /build
RUN chmod 777 /build

RUN apt-get update
COPY install-build-packages.sh /build
RUN /build/install-build-packages.sh
RUN rm /build/install-build-packages.sh
