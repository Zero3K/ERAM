@echo off
set TARGETNAME=eram
set NTDEBUGTYPE=coff
build.exe -cefw
if errorlevel 1 goto end
:end
start notepad build%BUILD_ALT_DIR%.log
