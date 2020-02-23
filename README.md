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

## Build

Als präferierte Ziel- und Buildplattform ist aus Gründen der Einfachheit zur Zeit für diesen Fork Linux vorgesehen.
Dennoch kann mit `qmake` oder dem QtCreator auch ein Build unter Windows durchgeführt werden.

### Build-Umgebung

* [Build unter Linux](BUILD_LINUX.md)
* [Build unter Windows](BUILD_WIN.md)

### Deployment

* [Deployment unter Linux](DEPLOYMENT_LINUX.md)
* [Deployment unter Windows](DEPLOYMENT_WIN.md)
