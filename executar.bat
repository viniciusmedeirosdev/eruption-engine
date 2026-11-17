@echo off
rem Vulcao SSAO - duplo clique para instalar o que faltar, compilar e executar.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0executar.ps1" %*
if errorlevel 1 pause
