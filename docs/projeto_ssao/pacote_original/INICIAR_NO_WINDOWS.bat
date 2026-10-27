@echo off
setlocal
cd /d "%~dp0"
where git >nul 2>&1
if errorlevel 1 (
  echo Git nao encontrado. Instale Git for Windows antes de continuar.
  pause
  exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\iniciar_windows.ps1"
if errorlevel 1 (
  echo Falha ao iniciar o clone. Verifique mensagens acima.
  pause
  exit /b 1
)
echo Pronto. Consulte LEIA_PRIMEIRO.md para compilar e testar no Ubuntu.
pause
