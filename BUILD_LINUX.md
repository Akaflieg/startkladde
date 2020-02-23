# Build unter Linux

Es sollte eine aktuelle Version des **GCC-Compilers** (`g++`) installiert sein und die aktuellste Version von **Qt 5**.

Außerdem wird die Bibliothek `libmariadb` oder `libmysqlclient` benötigt, mit Hilfe
derer auf die **MariaDB- oder MySQL-Datenbank** zugegriffen wird. Je nach Linux-Distribution müssen die Pfade zur Bibliotheksdatei und zu den Headers möglicherweise in der `startkladde.pro` im Bereich *External Dependencies* angepasst werden.

Darüber hinaus muss die **Ruby-Laufzeitumgebung** installiert sein, insbesondere wird die Templating-Engine __eRuby__ benötigt. Der Build wird nur dann klappen, wenn das Programm `erb` auf dem Terminal verfügbar ist.

Je nach Linux-Distribution sind die Headerdateien der Bibliotheken in einem extra Packet, sodass es beispielsweise nicht ausreicht `libmariadb` zu installieren, sondern zusätzlich `libmariadb-dev` oder ähnlich installiert werden muss.

### Kommandos

Die Startkladde kann mit Hilfe des `qmake`-Buildsystems von Qt gebaut werden:

```bash
$ cd <repository>
$ mkdir build
$ cd ../build
$ qmake ..
$ make
```
