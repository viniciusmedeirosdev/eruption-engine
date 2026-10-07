#!/usr/bin/env bash
# Comparacao usando os controles SSAO nativos do motor.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
MODE="${1:-on}"
if (( $# > 0 )); then shift; fi
case "$MODE" in
    on|off|capture) ;;
    *) echo "Uso: bash tools/ssao_demo.sh [on|off|capture] [argumentos da engine]" >&2; exit 2 ;;
esac
[[ -x ./eruption-engine ]] || { echo "Execute bash tools/build_ssao.sh primeiro." >&2; exit 1; }
[[ -s assets/external/parana_field/parana_field.glb ]] || {
    echo "Mapa ausente. Execute bash tools/fetch_demo_map.sh." >&2; exit 1;
}
export ERUPTION_TEST_SSAO_RADIUS="${SSAO_RADIUS:-12}"
export ERUPTION_TEST_SSAO_STRENGTH="${SSAO_STRENGTH:-0.8}"
# Mesma hora e vegetacao parada para facilitar a comparacao visual.
export ERUPTION_TEST_TIME=0.5
export ERUPTION_TEST_NO_WIND=1
export ERUPTION_TEST_NO_DOF=1
if [[ "$MODE" == capture ]]; then
    RUN_DIR="$ROOT/evidencias/$(date -u +%Y%m%dT%H%M%S)-$$"
    mkdir -p "$RUN_DIR"
    for state in off on; do
        if [[ "$state" == on ]]; then export ERUPTION_TEST_SSAO=1; else export ERUPTION_TEST_SSAO=0; fi
        ./eruption-engine --map parana_field "$@" --screenshot "$RUN_DIR/ssao-$state.png" > "$RUN_DIR/ssao-$state.log" 2>&1
        [[ -s "$RUN_DIR/ssao-$state.png" ]] || { echo "Captura ausente: consulte $RUN_DIR/ssao-$state.log" >&2; exit 1; }
    done
    printf 'SSAO radius=%s strength=%s\n' "$ERUPTION_TEST_SSAO_RADIUS" "$ERUPTION_TEST_SSAO_STRENGTH" > "$RUN_DIR/parametros.txt"
    echo "Capturas reais e logs: $RUN_DIR"
else
    if [[ "$MODE" == on ]]; then export ERUPTION_TEST_SSAO=1; else export ERUPTION_TEST_SSAO=0; fi
    exec ./eruption-engine --map parana_field "$@"
fi
