# Changelog

## Version 2.4.2 (April 2024)
* Bugfix: Gastflüge können in der Schnelleingabe wieder angelegt werden.

## Version 2.4.1 (März 2024)
* Bugfix: Beauftragender Fluglehrer wird korrekt abgespeichert.

## Version 2.4.0 (März 2024)
* Feature: Spitznamen können angegeben werden und werden bei der Personensuche berücksichtigt.
* Feature: Bei einsitzigen Schulungsflügen kann ein beauftragender Fluglehrer angegeben werden.

## Version 2.3.1 (April 2023)
* Build: Mit `qmake "CONFIG+=debpack"` kann unter Debian beim Build automatisch ein Debian-Paket erstellt und einem lokalen Repository in `$BUILD_DIR/../startkladde_ppa` hinzugefügt werden.
* Bugfix: Einige Bugs im Zusammenhang mit der Enter-Taste im Schnelleingabedialog wurden gefixt.
* Feature: Für den Vereinsflieger-Upload kann in den Einstellungen ein fester Account angegeben werden.
* Feature: Proxy-Skript. Es kann in den Einstellungen ein Skript angegeben werden, mit dessen Hilfe die Daten für den Vereinsflieger-Upload vor dem Hochladen manipuliert werden können (im JSON-Format). Damit kann man flugplatzspezifische Hacks umsetzen.
* Feature: Der Vereinsflieger-Upload kann nun auch nachträglich bearbeitete Flüge in Vereinsflieger aktualisieren

## [Version 2.2.0] (Feb. 2020)

* Breaking Change: Unter Linux werden mit `make install` nun alle zusätzlichen Dateien unter `/usr/share/startkladde` abgelegt.
* Feature: Flugupload zur Vereinsverwaltungplattform 'Vereinsflieger'
* Feature: Schnelleingabe-Dialog mit großen Bedienelementen und Schnellwahltasten sowie Auto-Vervollständigung basierend auf häufigen Piloten und Flugzeugen.
* Feature: Vom FLARM automatisch erkannte Starts und Landungen werden durch einen Ton signalisiert
* Feature: Als Datenbank ist nun MariaDB vorgesehen.
* Feature: Von Qt4 nach Qt5 portiert, für den Zugriff auf die serielle Schnittstelle wird die in Qt5 integrierte Bibliothek verwendet.
* Feature: Ein anonymer Modus wurde hinzugefügt. Bei Flügen in diesem Modus werden die Namen der Insassen nicht festgehalten, stattdessen nur die Anzahl Besatzungsmitglieder und Fluggäste.
* Feature: Hubschrauber wurden als Luftfahrzeugtyp hinzugefügt.
* Feature: METAR-Plugin mit einer korrekten URL versehen.
* Bugfix: Fehlende Übersetzungstext ergänzt.
* Regression: Batteriestandsanzeige entfernt, da die Bibliothek *libacpi* kein Linux-Standardpaket ist und schwer einzubinden. Kosten und Nutzen stehen deshalb in keinem Verhältnis, zumal der Batteriestand auch anderweitig im System eingesehen werden kann.

## 2.1.x (2012-06-16)

* Support of Flarm to detect start and landing automatically
* Support of Flarm to adjust system time based on GPS time
* Support of Flarm to show planes near the airfield in a table (Flarm Overview)
* Support of Flarm to show planes near the airfield on a map (Flarm Radar)
* Qwt6 library added to cmake
* Changed the term "Hauptflugbuch" (German translation only)
* Changed build system from make/qmake to cmake
* Bugfix: crash after canceling an update object operation from the main window
* Bugfix: (wrong) time zone displayed for time in main window
* Password protection for merging people
* Flight database window and CSV export
* Internationalization
* Password protect medical data (view and change)
* Add explicit database permission for connections from localhost (not included in "%"), fixes connection problem on Windows
* Launch method editor: option for towplanes: "select when creating a flight" or "specify registration"
* Changed tab order in settings window
* Improved messages when merging people
* Added tooltips to GUI 

## 2.1.0 (2010-10-24)

* Added function key shortcuts to menu entries
* Bugfix: newly created person/plane not stored correctly when creating flight 
* Improved libacpi state display 
* Bugfix: style ignored in label foreground colors 
* Bugfix: possible crash on refresh in ObjectListWindow
* New feature: Merging people
* Speedups in flight table display
* Allow disabling of "special" entries in ObjectSelectWindow
* ObjectSelectWindow uses ObjectModel
* Context menu in ObjectListWindow
* Space bar departs/lands flights without table button focus
* Select the same flight as before after refresh
* Caching of additional data of flights
* Bugfix: duplicate person name part entries in firstByLast/lastByFirst
	  name hashes in Cache on person change
* Bugfix: table button focus lost when current flight is edited
* Bugfix: table button focus lost when other cell of selected flight
	  is clicked
* Create flights/objects with double click on empty table space
* Don't edit with single clicks, independent of desktop settings
* Repeating a flight with Ctrl+Enter
* Improved flight color scheme
* Improved display of selected items in tables
* Improved keyboard navigation
* New feature: Medical validity checking
* MySQL 5.0 compatibility

## 2.0.7 (2010-07-29)

* Set time menu entry not shown on Windows
* Fixed time editor fields on Windows

## 2.0.6 (2010-07-29)

* Improved settings dialog
* New plugin architecture (ruby no longer required)
* Longitude correction and time zone for sunset plugin
* Include towflights in pilot and plane logs
* Removed demosystem code
* Checked return values of kvkbd system calls
* Made installation target in .pro Unix-only
* Renamed ChangeLog to changelog.txt (consistent with sk_web)

## 2.0.5 (2010-06-04)

* Added Ubuntu package

## 2.0.4 (2010-06-04)

* Cleaned up the build system

## 2.0.3 (2010-06-01)

* Ported plugins to Ruby
* Enabled plugins on Windows
* Minor bugfixes

## 2.0.2 (2010-05-31)

* Improved font handling

## 2.0.1 (2010-05-31)

* Compile on Windows

## 2.0

* Major rewrite

## 1.5

* Using QSettings for font and column width storage
* Support for kvkbd
* Added toolbar
* Added ACPI battery state display

## 1.4

* Bugfixes
* Added wetteronline.de radar image

## 1.3

* Added radar image
* Ported to Qt4

## 1.2

* Restart plugins by double-clicking the label

