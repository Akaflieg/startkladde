# How to Update Debian Package

## Dockerized Build

```bash
$ make-debian-package-docker.sh <ppa-dir> <e-mail>
```
The E-Mail is for signing the repository with a gpg key.

## Manually

This makes only sense if your build system is Debian.

On the build system either build the application with the special profile 'debpack' (by issuing `qmake "CONFIG+=debpack"`) or build the package manually after building the application:
```bash
$ make-debian-package.sh <source-dir> <build-dir> <ppa-dir>
```
The package version is automatically read from `startkladde.pro`.

Thereafter
```bash
$ sign-repo.sh <ppa-dir> <e-mail>
```
for signing some repo files with gpg. This need not be done on the build system, can also be a different system (where your gpg key is).
