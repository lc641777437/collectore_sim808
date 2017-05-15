rem CMD ,run "SIM800H32.bat all" or "SIM800H32.bat clean"
@echo off
rem set path=%CD%\build\winmake;%path%
set path=%CD%\build\winmake;%SystemRoot%\system32

echo. > output/log/output.log

make -f build\Makefile PROJ=SIM808M32 CFG=user %1
find "warnings," output/log/output.log
@pause