# Deployment unter Linux

Für das Ausführen werden die MariaDB- oder MySQL-Laufzeitbibliotheken sowie die von Qt5 benötigt. Einige Plugins benötigen zudem Ruby.

### Packages für Debian 11

Für Debian 11 _bullseye_ müssen die folgenden Packages installiert werden: `libqt5core5a libqt5multimedia5 libqt5serialport5 libqt5sql5-mysql libqt5printsupport5 libqt5xml5`.

### Kommandos

Die Installation der Startkladde ist nicht kompliziert:

```bash
$ cd <buildverzeichnis>
$ make install
```

Ebenso wie die Deinstallation:
```bash
$ cd <buildverzeichnis>
$ make uninstall
```
