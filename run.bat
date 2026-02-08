@echo off

if %1.==. start bin\debug
if not %1.==. start bin\release
cls