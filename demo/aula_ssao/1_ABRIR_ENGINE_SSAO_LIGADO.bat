@echo off
rem Demonstracao ao vivo: Eruption Engine com SSAO ligado no mapa parana_field.
rem F2 = painel de efeitos (iluminacao: SSAO liga/desliga, raio, intensidade, kernel academico)
rem F3 = desempenho (FPS / tempos de GPU).  Fechar a janela = sair.
rem Requer o build Windows (MSYS2 UCRT64). Ajuste MSYS2_BIN se o MSYS2 estiver em outro lugar.
if not defined MSYS2_BIN set "MSYS2_BIN=D:\msys64\ucrt64\bin"
set "PATH=%MSYS2_BIN%;%PATH%"
cd /d "%~dp0..\.."
set ERUPTION_TEST_SSAO=1
set ERUPTION_TEST_SSAO_RADIUS=12
set ERUPTION_TEST_SSAO_STRENGTH=0.8
set ERUPTION_TEST_TIME=0.5
set ERUPTION_TEST_NO_WIND=1
"%CD%\eruption-engine.exe" --map parana_field
