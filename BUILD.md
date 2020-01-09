# Windows

Für den Build wird die Installation des QT Creator mit einem aktuellen 64-Bit MinGW-Compiler empfohlen. 
Auf der [Open-Source-Seite](https://www.qt.io/download-open-source) von Qt
wird ein Online-Installer angeboten, in dem man komfortabel auswählen kann, 
welche Komponenten installiert werden sollen.

## Qt Creator

Beim ersten Öffnen des Projekts im Qt Creator wird eine Standard-Build-Konfiguration erstellt, die an sich gut funktioniert.Lediglich der 
Wert _Parallele Jobs_ in der Make-Konfiguration sollte auf 1 gestellt werden, da sonst unvollständig compiliert wird.
Außerdem kann man im Release-Profil den Haken bei _Qt Quick Compiler verwenden_ in der qmake-Konfiguration entfernen.

## MariaDB

Es werden mindestens die MariaDB-Client-Libaries benötigt.
Ist ohnehin schon ein MariaDB-Server installiert, sind diese im Programmverzeichnis mitgeliefert.
Andernfalls kann man auch den MariaDB C/C++ Connector für 64-Bit-Systeme installieren.
Anschließend muss in der `startkladde.pro` der Pfad `WIN32_MARIADB_INSTALLDIR` so gesetzt werden,
dass er auf das Installationsverzeichnis von MariaDB bzw. dem Connector zeigt. Die Ordner `lib` und `include`
müssen Unterverzeichnisse des betreffenden Ordners sein.

## Qt-MySQL-Plugin

## Ruby

Der Buildprozess benötigt das Konsolenprogramm `erb`. 
Indem Ruby installiert wird, sollte dieses auf der Konsole verfügbar werden.
