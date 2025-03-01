#!/bin/bash

# runtime dependencies of 'startkladde'
# for debian bookworm as of 2025-01-19

apt-get install \
	libqt6sql6-mysql \
	libqt6multimedia6 \
	libt6serialport6 \
	libqt6core6 \
	libqt6printsupport6 \
	libqt6sql6 \
	libqt6xml6
