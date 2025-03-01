#!/bin/bash

show_help() {
    echo "Usage: make-debian-package-docker.sh PPA_DIR EMAIL"
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
GPG_MAIL=$2

sudo docker buildx build -t startkladde-build . &&
	sudo docker run --rm -it \
	-e UID=$(id -u) -e GID=$(id -g) \
	-v "$(realpath ..):/startkladde:ro" \
	-v "$(realpath ${PPA_DIR}):/ppa" \
	startkladde-build \
	/bin/bash -c "cd /build && qmake6 /startkladde && make && /startkladde/debian/make-debian-package.sh /startkladde /build /ppa" &&
	./sign-repo.sh $PPA_DIR $GPG_MAIL

sudo docker image rm startkladde-build