#!/bin/bash

PPA_DIR=$1
GPG_MAIL=$2

rm -f .env &&
	echo "UID=$(id -u)" >> .env &&
	echo "GID=$(id -g)" >> .env &&
	echo "PPA_DIR=${PPA_DIR}" >> .env &&
	sudo docker-compose up --build --abort-on-container-exit &&
	rm -f .env &&
	./sign-repo.sh $PPA_DIR $GPG_MAIL
