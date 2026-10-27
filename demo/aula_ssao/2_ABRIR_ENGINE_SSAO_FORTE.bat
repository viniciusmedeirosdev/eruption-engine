@echo off
rem Demonstracao ao vivo: SSAO EXAGERADO (intensidade 2.0, raio 20) para ficar bem visivel no projetor.
rem F2 = painel de efeitos (iluminacao: SSAO liga/desliga, raio, intensidade, kernel academico)
if not defined MSYS2_BIN set "MSYS2_BIN=D:\msys64\ucrt64\bin"
set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%~dp0..\.."
set ERUPTION_TEST_SSAO=1
set ERUPTION_TEST_SSAO_RADIUS=20
set ERUPTION_TEST_SSAO_STRENGTH=2.0
set ERUPTION_TEST_TIME=0.5
set ERUPTION_TEST_NO_WIND=1
"%CD%\eruption-engine.exe" --map parana_field
