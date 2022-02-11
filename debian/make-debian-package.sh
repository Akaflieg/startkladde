#!/bin/bash

PROJ=$1
BUILD=$2
ARCH=amd64
REV=1

VERSION=$(cat $PROJ/startkladde.pro | grep "VERSION =" | sed -nE "s/^VERSION = ([0-9]+\.[0-9]+\.[0-9]+).*$/\1/p")
TARGET=$BUILD/startkladde_$VERSION-$REV_$ARCH

mkdir -p $TARGET
rm -r $TARGET
mkdir -p $TARGET/usr/bin
mkdir -p $TARGET/usr/share/startkladde
mkdir -p $TARGET/DEBIAN

cp $PROJ/debian/control.template $TARGET/DEBIAN/control
sed -i "s/Version: x.x.x/Version: ${VERSION}-${REV}/g" $TARGET/DEBIAN/control


cp $BUILD/startkladde $TARGET/usr/bin
cp -r $PROJ/plugins/info $TARGET/usr/share/startkladde/info
cp -r $PROJ/plugins/weather $TARGET/usr/share/startkladde/weather
cp $BUILD/translations/*.qm $TARGET/usr/share/startkladde

dpkg-deb --build $TARGET
