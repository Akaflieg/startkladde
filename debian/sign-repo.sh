#!/bin/sh

PPA_DIR=$1
EMAIL=$2

gpg --default-key "${EMAIL}" -abs -o - $PPA_DIR/Release > $PPA_DIR/Release.gpg
gpg --default-key "${EMAIL}" --clearsign -o - $PPA_DIR/Release > $PPA_DIR/InRelease
