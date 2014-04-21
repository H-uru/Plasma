@ECHO OFF
SET wdir=%~dp0
SET src=%wdir%prepare_env.ps1
PowerShell.exe -NoProfile -ExecutionPolicy Bypass -Command "& '%src%'";
