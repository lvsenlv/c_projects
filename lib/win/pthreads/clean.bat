@echo off

set TARGET_BASE_NAME=encrypt
set OBJ_DIR=.\object
set RELEASE_DIR=.\release

if exist 	*.a			(del /f /q *.a)
if exist	*.dll			(del /f /q *.dll)
if exist	*.exe			(del /f /q *.exe)
if exist	*.layout		(del /f /q *.layout)

if exist	%TARGET_BASE_NAME%*	(del /f /q %TARGET_BASE_NAME%*)
if exist	%OBJ_DIR%		(rd /s /q %OBJ_DIR%)
if exist	%RELEASE_DIR%		(rd /s /q %RELEASE_DIR%)
