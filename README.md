# Startkladde

In diesem Repository wohnt die von der Akaflieg Berlin ([@julianschick](https://github.com/julianschick)) gepflegte Version der Startkladde. Die Startkladde stammt ursprünglich von der Akaflieg Karlsruhe und wurde hier maßgeblich von Martin Herrman ([@deffi](https://github.com/deffi)) entwickelt. Eggert Ehmke ([@eehmke](https://github.com/eehmke)) vom FTV-Spandau hat die ungemein praktische automatische Starterfassung mit Hilfe eines FLARM-Gerätes beigetragen.

Ältere und teilweise veraltete sowie nicht mehr gepflegte Ressourcen zur Startkladde:

* [Ursprüngliche Sourceforge-Seite](http://startkladde.sourceforge.net/)
* [Github-Fork](https://github.com/startkladde/startkladde)
* [Noch ein Github-Fork](https://github.com/fb/startkladde)

Diese Version der Startkladde wurde weiter gepflegt, so dass sie auf einem aktuellen Linux-System mit einer aktuellen Version von GCC compiliert und gegen Qt5 gelinkt werden kann. Zudem wurden einige weitere Funktionalitäten hinzugefügt, u.a.:

* Flugdaten-Upload zu [Vereinsflieger](https://vereinsflieger.de)
* Schnelleingabe-Dialog mit großen Bedienelementen und Schnellwahltasten sowie Auto-Vervollständigung basierend auf häufigen Piloten und Flugzeugen

Siehe auch das [Changelog](CHANGELOG.md).

## Installation als Debian-Package aus unserem Repository

Hinzufügen des Repositories zu den Quellen:
```
$ curl -s "https://akaflieg.github.io/startkladde_ppa/KEY.gpg" | sudo apt-key add -
$ curl -s -o /etc/apt/sources.list.d/startkladde.list "https:/akaflieg.github.io/startkladde_ppa/startkladde.list"
```
Danach kann das Package mit `apt-get update` und `apt-get install startkladde` installiert werden. Ein MySQL- oder MariaDB-Datenbankserver muss gesondert installiert werden (`apt-get install mariadb-server`).

## Build

Als präferierte Ziel- und Buildplattform ist aus Gründen der Einfachheit zur Zeit für diesen Fork Linux vorgesehen.
Dennoch kann mit `qmake` oder dem QtCreator auch ein Build unter Windows durchgeführt werden.

* [Build unter Linux](BUILD_LINUX.md)
* [Build unter Windows](BUILD_WIN.md)

## Deployment

* [Deployment unter Linux](DEPLOYMENT_LINUX.md)
* [Deployment unter Windows](DEPLOYMENT_WIN.md)
* [Debian Packaging](debian/README.md)
