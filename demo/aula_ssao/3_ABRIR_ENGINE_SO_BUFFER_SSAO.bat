@echo off
rem Demonstracao ao vivo: mostra SO o buffer de oclusao (branco = sem oclusao, escuro = ocluido).
if not defined MSYS2_BIN set "MSYS2_BIN=D:\msys64\ucrt64\bin"
set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%~dp0..\.."
set ERUPTION_TEST_SSAO=1
set ERUPTION_TEST_SSAO_RADIUS=12
set ERUPTION_TEST_SSAO_STRENGTH=0.8
set ERUPTION_TEST_TIME=0.5
set ERUPTION_TEST_NO_WIND=1
set ERUPTION_TEST_PBR_DEBUG=128
"%CD%\eruption-engine.exe" --map parana_field
