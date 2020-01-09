# Windows

Um die Startkladde für Windows auszuliefern, müssen einige Dateien gesammelt werden
und einem Installationsverzeichnis gebündelt werden.
In diesem Repository der Startkladde gibt es dafür kein Skript,
da die Entwicklung sich auf Unixoide Betriebssysteme konzentriert.

Damit SSL-Downloads getätigt werden können, werden die OpenSSL-Bibliotheken
für Windows benötigt. Links zu den Downloads
können [hier](https://wiki.openssl.org/index.php/Binaries) gefunden werden.
Bei der Installation kann man wählen, wo die Libraries hinkopiert werden soll,
dort am besten das Programmverzeichnis auswählen. Anschließend kann man
die Libraries von dort wegkopieren.

Im folgenden eine Liste der notwendigen Dateien:

Dateiname | Quelle
-|-
`startkladde.exe` | `${RELEASE_BUILD}/release/startkladde.exe`
`Qt5Core.dll` | `${QT_DIR}/bin/Qt5Core.dll`
`Qt5Gui.dll` | `${QT_DIR}/bin/Qt5Gui.dll`
`Qt5Multimedia.dll` | `${QT_DIR}/bin/Qt5Multimedia.dll`
`Qt5Network.dll` | `${QT_DIR}/bin/Qt5Network.dll`
`Qt5PrintSupport.dll` | `${QT_DIR}/bin/Qt5PrintSupport.dll`
`Qt5SerialPort.dll` | `${QT_DIR}/bin/Qt5SerialPort.dll`
`Qt5Sql.dll` | `${QT_DIR}/bin/Qt5Sql.dll`
`Qt5Widgets.dll` | `${QT_DIR}/bin/Qt5Widgets.dll`
`Qt5Xml.dll` | `${QT_DIR}/bin/Qt5Xml.dll`
`translations/startkladde_*.qm` | `${RELEASE_BUILD}/translations/startkladde_*.qm`
`platforms/qwindows.dll` | ?
`libstdc++-6.dll` | ?
`libwinpthread-1.dll` | ?
`libgcc_s_seh-1.dll` | ?
`libcrypto-1_1-x64.dll` | ${OPENSSL_DIR}/libcrypto-1_1-x64.dll
`libssl-1?1-x64.dll` | ${OPENSSL_DIR}/libssl-1?1-x64.dll
