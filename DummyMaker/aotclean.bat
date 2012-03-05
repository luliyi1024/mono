@echo off
if not exist %1 (
	echo %1 not exist
) else (
	if exist %1.dll del /Q/F %1.dll
	if exist %~n1.def del /Q/F %~n1.def
	if exist %1.dummy del /Q/F %1.dummy
	if exist %1.dummy.log del /Q/F %1.dummy.log
)
@echo on