#!/bin/bash

# build dependencies of 'startkladde'
# for debian bookworm as of 2025-01-19

apt-get --assume-yes install \
	build-essential \
	make \
	qt6-base-dev \
	qt6-serialport-dev \
	qt6-multimedia-dev \
	qt6-l10n-tools \
	ruby \
	libmariadb-dev \
	apt-utils

