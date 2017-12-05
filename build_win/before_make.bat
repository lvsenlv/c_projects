@echo off

set OBJ_DIR=.\object
set RELEASE_DIR=.\release

if not exist	%OBJ_DIR% 	(mkdir %OBJ_DIR%)
if not exist	%RELEASE_DIR% 	(mkdir %RELEASE_DIR%)

