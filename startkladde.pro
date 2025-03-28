#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
#                                                                             #
#                       Startkladde qmake control file                        #
#                                                                             #
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#
#-----------------------------------------------------------------------------#

VERSION = 2.5.1
DEFINES += APPLICATION_VERSION=\\\"$$VERSION\\\"
CONFIG += c++20

#
# Basic settings
# 

TEMPLATE = app
TARGET = startkladde
INCLUDEPATH += .

#
#
#
QMAKE_CXXFLAGS += -Wall -Wextra -Wno-overloaded-virtual


#
# Parts of QT library to be included
#

QT += widgets sql network xml serialport multimedia printsupport

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000


#
# Informations for 'make install'
#

target.path = /usr/bin
documentation.path = /usr/share/startkladde
documentation.files = $${OUT_PWD}/translations/*
plugins.path = /usr/share/startkladde
plugins.files = $${PWD}/plugins/*
INSTALLS += target documentation plugins


#
# External Dependencies
#

# MySQL
WIN32_MARIADB_INSTALLDIR="C:\Program Files\MariaDB 10.4"
UNIX_MARIADB_PREFIX=/usr

win32:INCLUDEPATH += $${WIN32_MARIADB_INSTALLDIR}\include\mysql
win32:LIBS += $${WIN32_MARIADB_INSTALLDIR}\lib\libmariadb.lib
unix:INCLUDEPATH += $${UNIX_MARIADB_PREFIX}/include/mysql
unix:INCLUDEPATH += $${UNIX_MARIADB_PREFIX}/include/mariadb
unix:LIBS += -lmariadb

#
# Extra build steps
#

# CurrentSchema.cpp (DB migrations)
currentschema.commands = erb -T 1 schema_file=$$PWD/src/db/migrations/current_schema.yaml $$PWD/src/db/schema/CurrentSchema.cpp.erb > CurrentSchema.cpp
GENERATED_SOURCES += CurrentSchema.cpp
QMAKE_EXTRA_TARGETS += currentschema
PRE_TARGETDEPS += currentschema

# Translations
QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm translations/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all

# Debian Package
debpack {
    QMAKE_POST_LINK = $${PWD}/debian/make-debian-package.sh $${PWD} $${OUT_PWD} $${PWD}/../startkladde_ppa amd64
}


#
# Code, resource and translation files
#

HEADERS += src/accessor.h \
           src/data/TabularTextDocument.h \
           src/db/migrations/Migration_20191018135747_add_vfid.h \
           src/db/migrations/Migration_20200108164302_anonymous_columns.h \
           src/db/migrations/Migration_20220328172525_add_uploaded.h \
           src/db/migrations/Migration_20240323192110_add_supervisor.h \
           src/db/migrations/Migration_20250126193900_add_vfid_to_person.h \
           src/db/vfsync/vereinsfliegerflight.h \
           src/flightColor.h \
           src/FlightReference.h \
           src/gui/views/DateDelegate.h \
           src/itemDataRoles.h \
           src/Longitude.h \
           src/StorableException.h \
           src/text.h \
           src/util/WithSortkey.h \
           src/version.h \
           test/TestRunnerClient.h \
           test/textTest.h \
           src/algorithms/RectangleLayout.h \
           src/concurrent/DefaultQThread.h \
           src/concurrent/Returner.h \
           src/concurrent/synchronized.h \
           src/concurrent/threadUtil.h \
           src/concurrent/Waiter.h \
           src/config/Settings.h \
           src/container/Maybe.h \
           src/container/SkMultiHash.h \
           src/container/SortedSet.h \
           src/container/SortedSet_impl.h \
           src/data/Csv.h \
           src/db/Database.h \
           src/db/DatabaseInfo.h \
           src/db/dbId.h \
           src/db/DbManager.h \
           src/db/DbWorker.h \
           src/db/Query.h \
           src/flarm/Flarm.h \
           src/flarm/FlarmList.h \
           src/flarm/FlarmRecord.h \
           src/flarm/FlarmRecordModel.h \
           src/flarm/FlarmWindow.h \
           src/graphics/grid.h \
           src/graphics/SkMovie.h \
           src/gui/CompletionLineEdit.h \
           src/gui/dialogs.h \
           src/gui/PasswordCheck.h \
           src/gui/PasswordPermission.h \
           src/gui/SkCompleter.h \
           src/gui/SkDialog.h \
           src/gui/SkMainWindow.h \
           src/gui/Trainingsbarometer.h \
           src/gui/WidgetFader.h \
           src/i18n/LanguageChangeNotifier.h \
           src/i18n/LanguageConfiguration.h \
           src/i18n/notr.h \
           src/i18n/TranslationManager.h \
           src/io/AnsiColors.h \
           src/io/colors.h \
           src/io/console.h \
           src/io/SkProcess.h \
           src/io/TempDir.h \
           src/logging/messages.h \
           src/model/Entity.h \
           src/model/Flight.h \
           src/model/FlightBase.h \
           src/model/LaunchMethod.h \
           src/model/Person.h \
           src/model/PersonModel.h \
           src/model/Plane.h \
           src/net/Downloader.h \
           src/net/Network.h \
           src/net/TcpProxy.h \
           src/nmea/GprmcSentence.h \
           src/nmea/GpsTracker.h \
           src/nmea/Nmea.h \
           src/nmea/NmeaDecoder.h \
           src/nmea/NmeaSentence.h \
           src/nmea/PflaaSentence.h \
           src/numeric/Angle.h \
           src/numeric/Distance.h \
           src/numeric/GeoPosition.h \
           src/numeric/numeric.h \
           src/numeric/Scientific.h \
           src/numeric/Velocity.h \
           src/plugin/Plugin.h \
           src/qwt/SkPlotMagnifier.h \
           src/qwt/SkPlotPanner.h \
           src/statistics/LaunchMethodStatistics.h \
           src/statistics/PilotLog.h \
           src/statistics/PlaneLog.h \
           src/time/EventTimeTracker.h \
           src/util/bool.h \
           src/util/color.h \
           src/util/DestructionMonitor.h \
           src/util/double.h \
           src/util/environment.h \
           src/util/file.h \
           src/util/fileSystem.h \
           src/util/io.h \
           src/util/math.h \
           src/util/qDate.h \
           src/util/qHash.h \
           src/util/qList.h \
           src/util/qMargins.h \
           src/util/qPainter.h \
           src/util/qPointF.h \
           src/util/qRect.h \
           src/util/qRectF.h \
           src/util/qSize.h \
           src/util/qString.h \
           src/util/qTime.h \
           src/util/qTimer.h \
           src/util/qWidget.h \
           src/util/signal.h \
           src/util/time.h \
           src/concurrent/monitor/OperationCanceledException.h \
           src/concurrent/monitor/OperationMonitor.h \
           src/concurrent/monitor/OperationMonitorInterface.h \
           src/concurrent/monitor/SignalOperationMonitor.h \
           src/concurrent/monitor/SimpleOperationMonitor.h \
           src/db/cache/Cache.h \
           src/db/cache/CacheWorker.h \
           src/db/event/DbEvent.h \
           src/db/event/DbEventMonitor.h \
           src/db/interface/AbstractInterface.h \
           src/db/interface/DefaultInterface.h \
           src/db/interface/Interface.h \
           src/db/interface/InterfaceWorker.h \
           src/db/interface/ThreadSafeInterface.h \
           src/db/migration/Migration.h \
           src/db/migration/MigrationBuilder.h \
           src/db/migration/MigrationFactory.h \
           src/db/migration/Migrator.h \
           src/db/migration/MigratorWorker.h \
           src/db/migrations/Migration_20100214140000_initial.h \
           src/db/migrations/Migration_20100215000000_change_to_innodb.h \
           src/db/migrations/Migration_20100215172237_add_towpilot.h \
           src/db/migrations/Migration_20100215211913_drop_old_columns.h \
           src/db/migrations/Migration_20100215215320_convert_to_utf8.h \
           src/db/migrations/Migration_20100215221900_fix_data_types.h \
           src/db/migrations/Migration_20100216105657_remove_temp_tables.h \
           src/db/migrations/Migration_20100216122008_rename_tables.h \
           src/db/migrations/Migration_20100216124307_rename_columns.h \
           src/db/migrations/Migration_20100216135637_add_launch_methods.h \
           src/db/migrations/Migration_20100216171107_add_comments_to_users.h \
           src/db/migrations/Migration_20100216180053_full_plane_category.h \
           src/db/migrations/Migration_20100216190338_full_flight_type.h \
           src/db/migrations/Migration_20100217131516_flight_status_columns.h \
           src/db/migrations/Migration_20100314190344_full_flight_mode.h \
           src/db/migrations/Migration_20100427115235_add_indexes.h \
           src/db/migrations/Migration_20100726124616_add_medical_validity.h \
           src/db/migrations/Migration_20120612130803_add_flarm_id.h \
           src/db/migrations/Migration_20120620223948_add_flarmnet.h \
           src/db/result/CopiedResult.h \
           src/db/result/DefaultResult.h \
           src/db/result/Result.h \
           src/db/schema/CurrentSchema.h \
           src/db/schema/Schema.h \
           src/db/schema/SchemaDumper.h \
           src/db/vfsync/vereinsfliegersync.h \
           src/db/vfsync/vereinsfliegersyncworker.h \
           src/flarm/algorithms/FlarmIdCheck.h \
           src/flarm/algorithms/FlarmIdUpdate.h \
           src/flarm/algorithms/FlightLookup.h \
           src/flarm/algorithms/PlaneIdentification.h \
           src/flarm/algorithms/PlaneLookup.h \
           src/flarm/flarmMap/FlarmMapWidget.h \
           src/flarm/flarmMap/Kml.h \
           src/flarm/flarmMap/KmlReader.h \
           src/flarm/flarmNet/FlarmNetFile.h \
           src/flarm/flarmNet/FlarmNetHandler.h \
           src/flarm/flarmNet/FlarmNetRecord.h \
           src/flarm/flarmNet/FlarmNetRecordModel.h \
           src/flarm/flarmNet/FlarmNetWindow.h \
           src/gui/views/ReadOnlyItemDelegate.h \
           src/gui/views/SkItemDelegate.h \
           src/gui/views/SpecialIntDelegate.h \
           src/gui/views/SpinBoxCreator.h \
           src/gui/widgets/FlightTableView.h \
           src/gui/widgets/IconLabel.h \
           src/gui/widgets/LanguageComboBox.h \
           src/gui/widgets/LongitudeInput.h \
           src/gui/widgets/NotificationsLayout.h \
           src/gui/widgets/NotificationWidget.h \
           src/gui/widgets/PlotWidget.h \
           src/gui/widgets/SkComboBox.h \
           src/gui/widgets/SkLabel.h \
           src/gui/widgets/SkTableView.h \
           src/gui/widgets/SkTreeWidgetItem.h \
           src/gui/widgets/TableButton.h \
           src/gui/widgets/WeatherWidget.h \
           src/gui/windows/AboutDialog.h \
           src/gui/windows/ConfirmOverwritePersonDialog.h \
           src/gui/windows/CsvExportDialog.h \
           src/gui/windows/FlightWindow.h \
           src/gui/windows/FlightWizard.h \
           src/gui/windows/LaunchMethodSelectionWindow.h \
           src/gui/windows/LoginDialog.h \
           src/gui/windows/MainWindow.h \
           src/gui/windows/MonitorDialog.h \
           src/gui/windows/ObjectSelectWindow.h \
           src/gui/windows/ObjectSelectWindowBase.h \
           src/gui/windows/SettingsWindow.h \
           src/gui/windows/StatisticsWindow.h \
           src/gui/windows/SyncDialog.h \
           src/gui/windows/TrainingsBarometerDialog.h \
           src/gui/windows/WeatherDialog.h \
           src/io/dataStream/BackgroundDataStream.h \
           src/io/dataStream/DataStream.h \
           src/io/dataStream/FileDataStream.h \
           src/io/dataStream/SerialDataStream.h \
           src/io/dataStream/ManagedDataStream.h \
           src/io/dataStream/TcpDataStream.h \
           src/model/flightList/FlightModel.h \
           src/model/flightList/FlightProxyList.h \
           src/model/flightList/FlightSortFilterProxyModel.h \
           src/model/objectList/AbstractObjectList.h \
           src/model/objectList/AutomaticEntityList.h \
           src/model/objectList/ColumnInfo.h \
           src/model/objectList/EntityList.h \
           src/model/objectList/MutableObjectList.h \
           src/model/objectList/ObjectListModel.h \
           src/model/objectList/ObjectModel.h \
           src/plugin/factory/PluginFactory.h \
           src/plugin/info/InfoPlugin.h \
           src/plugin/info/InfoPluginSelectionDialog.h \
           src/plugin/info/InfoPluginSettingsPane.h \
           src/plugin/settings/PluginSettingsDialog.h \
           src/plugin/settings/PluginSettingsPane.h \
           src/plugin/weather/WeatherPlugin.h \
           src/plugins/weather/ExternalWeatherPlugin.h \
           src/plugins/weather/WetterOnlineAnimationPlugin.h \
           src/plugins/weather/WetterOnlineImagePlugin.h \
           src/db/interface/exceptions/AccessDeniedException.h \
           src/db/interface/exceptions/ConnectionFailedException.h \
           src/db/interface/exceptions/DatabaseDoesNotExistException.h \
           src/db/interface/exceptions/PingFailedException.h \
           src/db/interface/exceptions/QueryFailedException.h \
           src/db/interface/exceptions/SqlException.h \
           src/db/interface/exceptions/TransactionFailedException.h \
           src/db/schema/spec/ColumnSpec.h \
           src/db/schema/spec/IndexSpec.h \
           src/gui/windows/input/ChoiceDialog.h \
           src/gui/windows/input/DateInputDialog.h \
           src/gui/windows/input/DateTimeInputDialog.h \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorWindow.h \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.h \
           src/gui/windows/objectEditor/PersonEditorPane.h \
           src/gui/windows/objectEditor/PlaneEditorPane.h \
           src/gui/windows/objectList/FlightListWindow.h \
           src/gui/windows/objectList/ObjectListWindow.h \
           src/gui/windows/objectList/ObjectListWindowBase.h \
           src/gui/windows/objectList/PersonListWindow.h \
           src/plugins/info/external/ExternalInfoPlugin.h \
           src/plugins/info/external/ExternalInfoPluginSettingsPane.h \
           src/plugins/info/metar/MetarPlugin.h \
           src/plugins/info/metar/MetarPluginSettingsPane.h \
           src/plugins/info/sunset/SunsetCountdownPlugin.h \
           src/plugins/info/sunset/SunsetPluginBase.h \
           src/plugins/info/sunset/SunsetPluginSettingsPane.h \
           src/plugins/info/sunset/SunsetTimePlugin.h

FORMS += src/flarm/FlarmWindow.ui \
         src/flarm/flarmNet/FlarmNetWindow.ui \
         src/gui/widgets/IconLabel.ui \
         src/gui/widgets/LongitudeInput.ui \
         src/gui/windows/AboutDialog.ui \
         src/gui/windows/ConfirmOverwritePersonDialog.ui \
         src/gui/windows/CsvExportDialog.ui \
         src/gui/windows/FlightWindow.ui \
         src/gui/windows/FlightWizard.ui \
         src/gui/windows/LaunchMethodSelectionWindow.ui \
         src/gui/windows/LoginDialog.ui \
         src/gui/windows/MainWindow.ui \
         src/gui/windows/MonitorDialog.ui \
         src/gui/windows/ObjectSelectWindowBase.ui \
         src/gui/windows/SettingsWindow.ui \
         src/gui/windows/StatisticsWindow.ui \
         src/gui/windows/SyncDialog.ui \
         src/gui/windows/TrainingsBarometerDialog.ui \
         src/plugin/info/InfoPluginSelectionDialog.ui \
         src/plugin/info/InfoPluginSettingsPane.ui \
         src/plugin/settings/PluginSettingsDialog.ui \
         src/gui/windows/input/ChoiceDialog.ui \
         src/gui/windows/input/DateInputDialog.ui \
         src/gui/windows/input/DateTimeInputDialog.ui \
         src/gui/windows/objectEditor/LaunchMethodEditorPane.ui \
         src/gui/windows/objectEditor/ObjectEditorWindowBase.ui \
         src/gui/windows/objectEditor/PersonEditorPane.ui \
         src/gui/windows/objectEditor/PlaneEditorPane.ui \
         src/gui/windows/objectList/FlightListWindow.ui \
         src/gui/windows/objectList/ObjectListWindowBase.ui \
         src/plugins/info/external/ExternalInfoPluginSettingsPane.ui \
         src/plugins/info/metar/MetarPluginSettingsPane.ui \
         src/plugins/info/sunset/SunsetPluginSettingsPane.ui

SOURCES += src/flightColor.cpp \
           src/FlightReference.cpp \
           src/Longitude.cpp \
           src/data/TabularTextDocument.cpp \
           src/db/migrations/Migration_20191018135747_add_vfid.cpp \
           src/db/migrations/Migration_20200108164302_anonymous_columns.cpp \
           src/db/migrations/Migration_20220328172525_add_uploaded.cpp \
           src/db/migrations/Migration_20240323192110_add_supervisor.cpp \
           src/db/migrations/Migration_20250126193900_add_vfid_to_person.cpp \
           src/db/vfsync/vereinsfliegerflight.cpp \
           src/startkladde.cpp \
           src/StorableException.cpp \
           src/text.cpp \
           src/version.cpp \
           src/algorithms/RectangleLayout.cpp \
           src/concurrent/DefaultQThread.cpp \
           src/concurrent/Returner.cpp \
           src/concurrent/synchronized.cpp \
           src/concurrent/threadUtil.cpp \
           src/concurrent/Waiter.cpp \
           src/config/Settings.cpp \
           src/data/Csv.cpp \
           src/db/Database.cpp \
           src/db/DatabaseInfo.cpp \
           src/db/dbId.cpp \
           src/db/DbManager.cpp \
           src/db/DbWorker.cpp \
           src/db/Query.cpp \
           src/flarm/Flarm.cpp \
           src/flarm/FlarmList.cpp \
           src/flarm/FlarmRecord.cpp \
           src/flarm/FlarmRecordModel.cpp \
           src/flarm/FlarmWindow.cpp \
           src/graphics/grid.cpp \
           src/graphics/SkMovie.cpp \
           src/gui/CompletionLineEdit.cpp \
           src/gui/dialogs.cpp \
           src/gui/PasswordCheck.cpp \
           src/gui/PasswordPermission.cpp \
           src/gui/SkCompleter.cpp \
           src/gui/Trainingsbarometer.cpp \
           src/gui/WidgetFader.cpp \
           src/i18n/LanguageChangeNotifier.cpp \
           src/i18n/LanguageConfiguration.cpp \
           src/i18n/TranslationManager.cpp \
           src/io/AnsiColors.cpp \
           src/io/console.cpp \
           src/io/SkProcess.cpp \
           src/io/TempDir.cpp \
           src/logging/messages.cpp \
           src/model/Entity.cpp \
           src/model/Flight.cpp \
           src/model/Flight_Mode.cpp \
           src/model/Flight_Type.cpp \
           src/model/FlightBase.cpp \
           src/model/LaunchMethod.cpp \
           src/model/Person.cpp \
           src/model/PersonModel.cpp \
           src/model/Plane.cpp \
           src/net/Downloader.cpp \
           src/net/Network.cpp \
           src/net/TcpProxy.cpp \
           src/nmea/GprmcSentence.cpp \
           src/nmea/GpsTracker.cpp \
           src/nmea/Nmea.cpp \
           src/nmea/NmeaDecoder.cpp \
           src/nmea/NmeaSentence.cpp \
           src/nmea/PflaaSentence.cpp \
           src/numeric/Angle.cpp \
           src/numeric/Distance.cpp \
           src/numeric/GeoPosition.cpp \
           src/numeric/numeric.cpp \
           src/numeric/Scientific.cpp \
           src/numeric/Velocity.cpp \
           src/plugin/Plugin.cpp \
           src/qwt/SkPlotMagnifier.cpp \
           src/qwt/SkPlotPanner.cpp \
           src/statistics/LaunchMethodStatistics.cpp \
           src/statistics/PilotLog.cpp \
           src/statistics/PlaneLog.cpp \
           src/time/EventTimeTracker.cpp \
           src/util/bool.cpp \
           src/util/color.cpp \
           src/util/DestructionMonitor.cpp \
           src/util/double.cpp \
           src/util/environment.cpp \
           src/util/file.cpp \
           src/util/fileSystem.cpp \
           src/util/io.cpp \
           src/util/qDate.cpp \
           src/util/qHash.cpp \
           src/util/qMargins.cpp \
           src/util/qPainter.cpp \
           src/util/qPointF.cpp \
           src/util/qRect.cpp \
           src/util/qRectF.cpp \
           src/util/qSize.cpp \
           src/util/qString.cpp \
           src/util/qTime.cpp \
           src/util/qTimer.cpp \
           src/util/qWidget.cpp \
           src/util/signal.cpp \
           src/util/time.cpp \
           src/concurrent/monitor/OperationCanceledException.cpp \
           src/concurrent/monitor/OperationMonitor.cpp \
           src/concurrent/monitor/OperationMonitorInterface.cpp \
           src/concurrent/monitor/SignalOperationMonitor.cpp \
           src/concurrent/monitor/SimpleOperationMonitor.cpp \
           src/db/cache/Cache.cpp \
           src/db/cache/Cache_hashUpdates.cpp \
           src/db/cache/Cache_lookup.cpp \
           src/db/cache/CacheWorker.cpp \
           src/db/event/DbEvent.cpp \
           src/db/event/DbEventMonitor.cpp \
           src/db/interface/AbstractInterface.cpp \
           src/db/interface/DefaultInterface.cpp \
           src/db/interface/Interface.cpp \
           src/db/interface/InterfaceWorker.cpp \
           src/db/interface/ThreadSafeInterface.cpp \
           src/db/migration/Migration.cpp \
           src/db/migration/MigrationFactory.cpp \
           src/db/migration/Migrator.cpp \
           src/db/migration/MigratorWorker.cpp \
           src/db/migrations/Migration_20100214140000_initial.cpp \
           src/db/migrations/Migration_20100215000000_change_to_innodb.cpp \
           src/db/migrations/Migration_20100215172237_add_towpilot.cpp \
           src/db/migrations/Migration_20100215211913_drop_old_columns.cpp \
           src/db/migrations/Migration_20100215215320_convert_to_utf8.cpp \
           src/db/migrations/Migration_20100215221900_fix_data_types.cpp \
           src/db/migrations/Migration_20100216105657_remove_temp_tables.cpp \
           src/db/migrations/Migration_20100216122008_rename_tables.cpp \
           src/db/migrations/Migration_20100216124307_rename_columns.cpp \
           src/db/migrations/Migration_20100216135637_add_launch_methods.cpp \
           src/db/migrations/Migration_20100216171107_add_comments_to_users.cpp \
           src/db/migrations/Migration_20100216180053_full_plane_category.cpp \
           src/db/migrations/Migration_20100216190338_full_flight_type.cpp \
           src/db/migrations/Migration_20100217131516_flight_status_columns.cpp \
           src/db/migrations/Migration_20100314190344_full_flight_mode.cpp \
           src/db/migrations/Migration_20100427115235_add_indexes.cpp \
           src/db/migrations/Migration_20100726124616_add_medical_validity.cpp \
           src/db/migrations/Migration_20120612130803_add_flarm_id.cpp \
           src/db/migrations/Migration_20120620223948_add_flarmnet.cpp \
           src/db/result/CopiedResult.cpp \
           src/db/result/DefaultResult.cpp \
           src/db/schema/Schema.cpp \
           src/db/schema/SchemaDumper.cpp \
           src/db/vfsync/vereinsfliegersync.cpp \
           src/db/vfsync/vereinsfliegersyncworker.cpp \
           src/flarm/algorithms/FlarmIdCheck.cpp \
           src/flarm/algorithms/FlarmIdUpdate.cpp \
           src/flarm/algorithms/FlightLookup.cpp \
           src/flarm/algorithms/PlaneIdentification.cpp \
           src/flarm/algorithms/PlaneLookup.cpp \
           src/flarm/flarmMap/FlarmMapWidget.cpp \
           src/flarm/flarmMap/Kml.cpp \
           src/flarm/flarmMap/KmlReader.cpp \
           src/flarm/flarmNet/FlarmNetFile.cpp \
           src/flarm/flarmNet/FlarmNetHandler.cpp \
           src/flarm/flarmNet/FlarmNetRecord.cpp \
           src/flarm/flarmNet/FlarmNetRecordModel.cpp \
           src/flarm/flarmNet/FlarmNetWindow.cpp \
           src/gui/views/ReadOnlyItemDelegate.cpp \
           src/gui/views/SkItemDelegate.cpp \
           src/gui/views/SpecialIntDelegate.cpp \
           src/gui/views/SpinBoxCreator.cpp \
           src/gui/widgets/FlightTableView.cpp \
           src/gui/widgets/IconLabel.cpp \
           src/gui/widgets/LanguageComboBox.cpp \
           src/gui/widgets/LongitudeInput.cpp \
           src/gui/widgets/NotificationsLayout.cpp \
           src/gui/widgets/NotificationWidget.cpp \
           src/gui/widgets/PlotWidget.cpp \
           src/gui/widgets/SkComboBox.cpp \
           src/gui/widgets/SkLabel.cpp \
           src/gui/widgets/SkTableView.cpp \
           src/gui/widgets/SkTreeWidgetItem.cpp \
           src/gui/widgets/TableButton.cpp \
           src/gui/widgets/WeatherWidget.cpp \
           src/gui/windows/AboutDialog.cpp \
           src/gui/windows/ConfirmOverwritePersonDialog.cpp \
           src/gui/windows/CsvExportDialog.cpp \
           src/gui/windows/FlightWindow.cpp \
           src/gui/windows/FlightWizard.cpp \
           src/gui/windows/LaunchMethodSelectionWindow.cpp \
           src/gui/windows/LoginDialog.cpp \
           src/gui/windows/MainWindow.cpp \
           src/gui/windows/MonitorDialog.cpp \
           src/gui/windows/ObjectSelectWindow.cpp \
           src/gui/windows/ObjectSelectWindowBase.cpp \
           src/gui/windows/SettingsWindow.cpp \
           src/gui/windows/StatisticsWindow.cpp \
           src/gui/windows/SyncDialog.cpp \
           src/gui/windows/TrainingsBarometerDialog.cpp \
           src/gui/windows/WeatherDialog.cpp \
           src/io/dataStream/BackgroundDataStream.cpp \
           src/io/dataStream/DataStream.cpp \
           src/io/dataStream/FileDataStream.cpp \
           src/io/dataStream/ManagedDataStream.cpp \
           src/io/dataStream/SerialDataStream.cpp \
           src/io/dataStream/TcpDataStream.cpp \
           src/model/flightList/FlightModel.cpp \
           src/model/flightList/FlightProxyList.cpp \
           src/model/flightList/FlightSortFilterProxyModel.cpp \
           src/plugin/factory/PluginFactory.cpp \
           src/plugin/info/InfoPlugin.cpp \
           src/plugin/info/InfoPluginSelectionDialog.cpp \
           src/plugin/info/InfoPluginSettingsPane.cpp \
           src/plugin/settings/PluginSettingsDialog.cpp \
           src/plugin/settings/PluginSettingsPane.cpp \
           src/plugin/weather/WeatherPlugin.cpp \
           src/plugins/weather/ExternalWeatherPlugin.cpp \
           src/plugins/weather/WetterOnlineAnimationPlugin.cpp \
           src/plugins/weather/WetterOnlineImagePlugin.cpp \
           src/db/interface/exceptions/AccessDeniedException.cpp \
           src/db/interface/exceptions/ConnectionFailedException.cpp \
           src/db/interface/exceptions/DatabaseDoesNotExistException.cpp \
           src/db/interface/exceptions/PingFailedException.cpp \
           src/db/interface/exceptions/QueryFailedException.cpp \
           src/db/interface/exceptions/SqlException.cpp \
           src/db/interface/exceptions/TransactionFailedException.cpp \
           src/db/schema/spec/ColumnSpec.cpp \
           src/db/schema/spec/IndexSpec.cpp \
           src/gui/windows/input/ChoiceDialog.cpp \
           src/gui/windows/input/DateInputDialog.cpp \
           src/gui/windows/input/DateTimeInputDialog.cpp \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.cpp \
           src/gui/windows/objectEditor/PersonEditorPane.cpp \
           src/gui/windows/objectEditor/PlaneEditorPane.cpp \
           src/gui/windows/objectList/FlightListWindow.cpp \
           src/gui/windows/objectList/ObjectListWindow.cpp \
           src/gui/windows/objectList/ObjectListWindowBase.cpp \
           src/gui/windows/objectList/PersonListWindow.cpp \
           src/plugins/info/external/ExternalInfoPlugin.cpp \
           src/plugins/info/external/ExternalInfoPluginSettingsPane.cpp \
           src/plugins/info/metar/MetarPlugin.cpp \
           src/plugins/info/metar/MetarPluginSettingsPane.cpp \
           src/plugins/info/sunset/SunsetCountdownPlugin.cpp \
           src/plugins/info/sunset/SunsetPluginBase.cpp \
           src/plugins/info/sunset/SunsetPluginSettingsPane.cpp \
           src/plugins/info/sunset/SunsetTimePlugin.cpp

RESOURCES += config/startkladde.qrc

TRANSLATIONS += translations/startkladde_bork.ts \
                translations/startkladde_de.ts
