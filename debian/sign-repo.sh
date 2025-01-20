#!/bin/sh

show_help() {
    echo "Usage: sign-repo.sh PPA_DIR EMAIL"
    echo
    echo "This script requires two parameters:"
    echo "  PPA_DIR - The directory containing the package repo (ppa)"
    echo "  EMAIL   - The email address of the private GPG key to be used"
}

if [ "$#" -ne 2 ]; then
    echo "Error: Missing required parameters."
    show_help
    exit 1
fi

PPA_DIR=$1
EMAIL=$2

gpg --default-key "${EMAIL}" -abs -o - $PPA_DIR/Release > $PPA_DIR/Release.gpg
gpg --default-key "${EMAIL}" --clearsign -o - $PPA_DIR/Release > $PPA_DIR/InRelease
