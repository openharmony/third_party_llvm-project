@ECHO OFF
SET PYTHONPATH=%~dp0..\script
%~dp0lldb.exe %*
EXIT /B%ERRORLEVEL%
