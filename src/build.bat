@echo off
set DIRS=.\obj%BUILD_ALT_DIR%\i386
set TARGETNAME=eramnt
if defined BUILD_ALT_DIR goto NT5
set DIRS=.\obj\i386\%DDKBUILDENV%
:NT5
set NTDEBUGTYPE=coff
build -cefw
if errorlevel 1 goto end
rebase -b 0x10000 -x %DIRS% %DIRS%\%TARGETNAME%.sys
copy %DIRS%\%TARGETNAME%.dbg %SYSTEMROOT%\symbols\*.*
copy %DIRS%\%TARGETNAME%.sys .\*.*
:end
start notepad build%BUILD_ALT_DIR%.log
