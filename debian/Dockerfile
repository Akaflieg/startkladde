FROM debian:bookworm

RUN apt-get update
RUN apt-get --assume-yes install build-essential make qt6-base-dev qt6-serialport-dev qt6-multimedia-dev qt6-l10n-tools ruby libmariadb-dev apt-utils

RUN mkdir /build
RUN chmod 777 /build
