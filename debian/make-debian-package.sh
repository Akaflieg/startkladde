#!/bin/bash

PROJ=$1
BUILD=$2
VERSION=$3
#VERSION=$(cat $PROJ/startkladde.pro | grep "VERSION =" | sed -nE "s/^VERSION = ([0-9]+\.[0-9]+\.[0-9]+).*$/\1/p")
ARCH=$4

TARGET=$BUILD/startkladde_${VERSION}_${ARCH}

mkdir -p $TARGET
mkdir -p $TARGET/usr/bin
mkdir -p $TARGET/usr/share/startkladde
mkdir -p $TARGET/DEBIAN

cp $PROJ/debian/control.template $TARGET/DEBIAN/control
sed -i "s/Version: x.x.x/Version: ${VERSION}/g" $TARGET/DEBIAN/control


cp $BUILD/startkladde $TARGET/usr/bin
cp -r $PROJ/plugins/info $TARGET/usr/share/startkladde/info
cp -r $PROJ/plugins/weather $TARGET/usr/share/startkladde/weather
cp $BUILD/translations/*.qm $TARGET/usr/share/startkladde

dpkg-deb --build $TARGET
