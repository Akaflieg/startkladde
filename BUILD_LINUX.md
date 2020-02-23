Es sollte eine aktuelle Version des **GCC-Compilers** (`g++`) installiert sein und die aktuellste Version von **Qt 5**. Außerdem wird die Bibliothek `libmysqlclient` benötigt, mit Hilfe
derer auf die **MySQL- oder MariaDB-Datenbank** zugegriffen wird.


Darüber hinaus muss die **Ruby-Laufzeitumgebung** installiert sein, insbesondere wird die Templating-Engine __eRuby__ benötigt. Der Build wird nur dann klappen, wenn das Programm `erb` auf dem Terminal verfügbar ist.

### Kommandos

Die Startkladde kann mit Hilfe des `qmake`-Buildsystems von Qt gebaut werden:

```bash
$ cd <repository>
$ mkdir build
$ cd ../build
$ qmake ..
$ make
```
