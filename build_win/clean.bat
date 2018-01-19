@echo off

set OBJ_DIR=.\object
set RELEASE_DIR=.\release
set LIB_DIR=..\lib\win

if exist 	*.a			(del /f /q *.a)
if exist	*.dll			(del /f /q *.dll)
if exist	*.exe			(del /f /q *.exe)
if exist	*.layout		(del /f /q *.layout)
if exist	encrypt_win*		(del /f /q encrypt_win*)

if exist	%OBJ_DIR%		(rd /s /q %OBJ_DIR%)
if exist	%RELEASE_DIR%		(rd /s /q %RELEASE_DIR%)

if exist 	%LIB_DIR%\*.a		(del /f /q %LIB_DIR%\*.a)
