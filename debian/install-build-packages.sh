#!/bin/bash

# build dependencies of 'startkladde'
# for debian bullseye as of 2022-02-11

apt-get install \
build-essential \
qtbase5-dev \
libqt5serialport5-dev \
qtmultimedia5-dev \
qttools5-dev-tools \
ruby \
libmariadb-dev
