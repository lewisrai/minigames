@echo off

set compiler=gcc
set stdlibversion=-std=c17
set name=Minigames
set linkedlibs=-lgdi32

if %1.==. (
	set binpath=bin\debug
	set argument1=-g -Og -municode
	set argument2=-municode
)
if not %1.==. (
	set binpath=bin\release
	set argument1=-O1 -municode
	set argument2=-s -mwindows -municode
)

rmdir "%binpath%" /s /q
mkdir "%binpath%"

windres include\manifest.rc -o manifest.o

echo Building ...
%compiler% -c src\*.c %stdlibversion% %argument1% -Wall -m64 -I include
%compiler% *.o -o %binpath%\%name% %argument2% -L lib %linkedlibs%
echo ... Build Complete

del *.o
start %binpath%
pause
cls
