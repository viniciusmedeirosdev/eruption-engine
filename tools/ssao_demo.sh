#!/usr/bin/env bash
# Comparacao usando os controles SSAO nativos do motor.
#   bash tools/ssao_demo.sh [on|off|capture] [argumentos da engine]
# Variaveis opcionais: SSAO_RADIUS, SSAO_STRENGTH, SSAO_QUALITY (engine|academic),
# SSAO_CAPTURE_FRAME (quadro da captura no modo capture, padrao 1500).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
MODE="${1:-on}"
if (( $# > 0 )); then shift; fi
case "$MODE" in
    on|off|capture) ;;
    *) echo "Uso: bash tools/ssao_demo.sh [on|off|capture] [argumentos da engine]" >&2; exit 2 ;;
esac
# Linux: ./eruption-engine; Windows (MSYS2/MinGW): ./eruption-engine.exe
BIN=./eruption-engine
[[ -f "$BIN" ]] || BIN=./eruption-engine.exe
[[ -f "$BIN" ]] || { echo "Execute bash tools/build_ssao.sh primeiro." >&2; exit 1; }
[[ -s assets/external/parana_field/parana_field.glb ]] || {
    echo "Mapa ausente. Execute bash tools/fetch_demo_map.sh." >&2; exit 1;
}
export ERUPTION_TEST_SSAO_RADIUS="${SSAO_RADIUS:-12}"
export ERUPTION_TEST_SSAO_STRENGTH="${SSAO_STRENGTH:-0.8}"
# engine = 12 amostras + blur 3x3 (padrao); academic = 32 amostras + blur 5x5.
export ERUPTION_TEST_SSAO_QUALITY="${SSAO_QUALITY:-engine}"
# Mesma hora e vegetacao parada para facilitar a comparacao visual.
export ERUPTION_TEST_TIME=0.5
export ERUPTION_TEST_NO_WIND=1
export ERUPTION_TEST_NO_DOF=1
if [[ "$MODE" == capture ]]; then
    # Par OFF/ON deterministico (medido: duas execucoes OFF iguais bit a bit):
    #  - exposicao automatica desligada: ela compensava o escurecimento do SSAO
    #    e o par chegava a sair MAIS CLARO com o efeito ligado;
    #  - sem jitter subpixel e captura no quadro 1500 (o streaming de texturas
    #    ainda variava no quadro 600 padrao).
    export ERUPTION_TEST_AUTO_EXPOSURE=0
    export ERUPTION_TEST_NO_JITTER=1
    export ERUPTION_TEST_SCREENSHOT_FRAME="${SSAO_CAPTURE_FRAME:-1500}"
    RUN_DIR="$ROOT/evidencias/$(date -u +%Y%m%dT%H%M%S)-$$"
    mkdir -p "$RUN_DIR"
    for state in off on; do
        if [[ "$state" == on ]]; then export ERUPTION_TEST_SSAO=1; else export ERUPTION_TEST_SSAO=0; fi
        "$BIN" --map parana_field "$@" --screenshot "$RUN_DIR/ssao-$state.png" > "$RUN_DIR/ssao-$state.log" 2>&1
        [[ -s "$RUN_DIR/ssao-$state.png" ]] || { echo "Captura ausente: consulte $RUN_DIR/ssao-$state.log" >&2; exit 1; }
    done
    {
        printf 'SSAO radius=%s strength=%s quality=%s\n' \
            "$ERUPTION_TEST_SSAO_RADIUS" "$ERUPTION_TEST_SSAO_STRENGTH" "$ERUPTION_TEST_SSAO_QUALITY"
        printf 'mapa=parana_field time=%s screenshot_frame=%s auto_exposure=0 jitter=0 wind=0 dof=0\n' \
            "$ERUPTION_TEST_TIME" "$ERUPTION_TEST_SCREENSHOT_FRAME"
        printf 'argumentos extras: %s\n' "$*"
        printf 'commit=%s\n' "$(git rev-parse HEAD 2>/dev/null || echo desconhecido)"
        printf 'data_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
        grep -h 'GPU em uso' "$RUN_DIR/ssao-on.log" | sed 's/\x1b\[[0-9;]*m//g' | head -1 || true
    } > "$RUN_DIR/parametros.txt"
    # Metricas e mapa de diferenca (opcional: requer Python com Pillow e NumPy).
    PY="$(command -v python3 || command -v python || true)"
    if [[ -n "$PY" ]] && "$PY" -c 'import PIL, numpy' 2>/dev/null; then
        "$PY" tools/ssao_compare.py "$RUN_DIR/ssao-off.png" "$RUN_DIR/ssao-on.png" "$RUN_DIR" > /dev/null
    fi
    if grep -qiE 'VUID|Validation Error' "$RUN_DIR"/ssao-*.log; then
        echo "AVISO: mensagens de validacao Vulkan nos logs de $RUN_DIR" >&2
    fi
    echo "Capturas reais e logs: $RUN_DIR"
else
    if [[ "$MODE" == on ]]; then export ERUPTION_TEST_SSAO=1; else export ERUPTION_TEST_SSAO=0; fi
    exec "$BIN" --map parana_field "$@"
fi
