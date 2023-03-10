#!/bin/bash

SOURCES_DIR=$1
BUILD_DIR=$2
PPA_DIR=$3

VERSION=$(cat $SOURCES_DIR/startkladde.pro | grep "VERSION =" | sed -nE "s/^VERSION = ([0-9]+\.[0-9]+\.[0-9]+).*$/\1/p")
ARCH=amd64
GITHUB_USERNAME="Akaflieg"

# Make binary

cd $BUILD_DIR
cmake $SOURCES_DIR
make

# Make package

TARGET=$BUILD_DIR/startkladde_${VERSION}_${ARCH}

mkdir -p $TARGET
mkdir -p $TARGET/usr/bin
mkdir -p $TARGET/usr/share/startkladde
mkdir -p $TARGET/DEBIAN

cp $SOURCES_DIR/debian/control.template $TARGET/DEBIAN/control
sed -i "s/Version: x.x.x/Version: ${VERSION}/g" $TARGET/DEBIAN/control


cp $BUILD_DIR/startkladde $TARGET/usr/bin
cp -r $SOURCES_DIR/plugins/info $TARGET/usr/share/startkladde/info
cp -r $SOURCES_DIR/plugins/weather $TARGET/usr/share/startkladde/weather
cp $BUILD_DIR/translations/*.qm $TARGET/usr/share/startkladde

dpkg-deb --build $TARGET

# Copy package to PPA

cp ${TARGET}.deb $PPA_DIR

# Update PPA lists

cd $PPA_DIR

dpkg-scanpackages --multiversion . > Packages
gzip -k -f Packages

apt-ftparchive release . > Release
echo "deb https://${GITHUB_USERNAME}.github.io/startkladde_ppa ./" > startkladde.list

echo "<<< DONE >>>"
echo "Do not forget to sign the PPA files with GPG."
