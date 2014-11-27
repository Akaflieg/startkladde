@echo off
echo "Startkladde debugging batch file"
echo.
echo.
echo.
cd ..
del startkladde.log
.\startkladde.exe --no-full-screen -q >startkladde.log
type startkladde.log

