@echo off

set OBJ_DIR=.\object

if exist	*.a		(del /f /q *.a)
if exist	*.dll		(del /f /q *.dll)
if exist	*.exe		(del /f /q *.exe)
if exist	*.layout		(del /f /q *.layout)
if exist	%OBJ_DIR%\*.o	(del /s /q /f %OBJ_DIR%\*.o)
