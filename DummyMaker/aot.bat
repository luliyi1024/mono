@echo off
if not exist %1 (
	echo %1 not exist
) else (
	mono.exe --aot %1 > %~n1.def
	if not exist %~n1.def (echo cannot find %~n1.def) else (dummymaker.exe %1 %~n1.def > %1.dummy.log)
)
@echo on