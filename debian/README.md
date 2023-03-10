# How to Update Debian Package

On the build system do
```bash
$ make_debian_package.sh <source-dir> <build-dir> <ppa-dir>
```
The package version is automatically read from `startkladde.pro`.

Thereafter
```bash
$ sign_repo.sh <ppa-dir> <e-mail>
```
for signing some repo files with gpg. This need not be done on the build system, can also be a different system (where your gpg key is).