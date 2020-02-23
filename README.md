# Startkladde

In diesem Repository wohnt die von der Akaflieg Berlin ([@julianschick](https://github.com/julianschick)) gepflegte Version der Startkladde. Die Startkladde stammt ursprünglich von der Akaflieg Karlsruhe und wurde hier maßgeblich von Martin Herrman ([@deffi](https://github.com/deffi)) entwickelt. Eggert Ehmke ([@eehmke](https://github.com/eehmke)) vom FTV-Spandau hat die ungemein praktische automatische Starterfassung mit Hilfe eines FLARM-Gerätes beigetragen.

Ältere und teilweise veraltete sowie nicht mehr gepflegte Ressourcen zur Startkladde:

* [Ursprüngliche Sourceforge-Seite](http://startkladde.sourceforge.net/)
* [Github-Fork](https://github.com/startkladde/startkladde)
* [Noch ein Github-Fork](https://github.com/fb/startkladde)

Diese Version der Startkladde wurde weiter gepflegt, so dass sie auf einem aktuellen Linux-System mit einer aktuellen Version von GCC compiliert und gegen Qt 5 gelinkt werden kann. Zudem wurden einige weitere Funktionalitäten hinzugefügt:

* Flugdaten-Upload zu [Vereinsflieger](https://vereinsflieger.de)
* Schnelleingabe-Dialog mit großen Bedienelementen und Schnellwahltasten sowie Auto-Vervollständigung basierend auf häufigen Piloten und Flugzeugen
* Aktualisierung der Wetterplugins

Siehe auch das [Changelog](CHANGELOG.md).

## Build

Als Ziel- und Buildplattform wird von diesem Fork aus Gründen der Einfachheit zur Zeit nur Linux unterstützt. Da der Build mit `qmake` durchgeführt wird,
sollte eine Portierung jedoch nicht allzu schwierig sein.

### Build-Umgebung

* [Build unter Windows](BUILD_WIN.md)
* Build unter Linux (siehe unten)

Es sollte eine aktuelle Version des **GCC-Compilers** (`g++`) installiert sein und die aktuellste Version von **Qt 5**. Außerdem wird die Bibliothek `libmysqlclient` benötigt, mit Hilfe
derer auf die **MySQL- oder MariaDB-Datenbank** zugegriffen wird.

### Deployment

* [Deployment unter Windows](DEPLOYMENT_WIN.md)
* Deployment unter Linux (siehe unten)

Die Installation ist auch nicht kompliziert:

```bash
$ make install
```

Ebenso wie die Deinstallation:
```bash
$ make uninstall
```
