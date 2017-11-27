@echo off

for /r .\ %%i in (*.o) do (
   del /f "%%i"
)

if exist	 *.a 	(del /f *.a)
if exist 	*.dll 	(del /f *.dll)
if exist	 *.exe 	(del /f *.exe)
if exist 	*.layout 	(del /f *.layout)
