@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>nul
if %errorlevel%==0 (
    py -3 "scripts\instalar_ordens_no_fork.py" %*
) else (
    python "scripts\instalar_ordens_no_fork.py" %*
)
if errorlevel 1 (
    echo Confira a mensagem de erro acima.
    exit /b 1
)
endlocal
