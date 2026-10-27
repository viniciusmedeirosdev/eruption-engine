#!/usr/bin/env bash
set -Eeuo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO="$BASE/eruption-engine"
[[ -d "$REPO/.git" ]] || { echo 'Clone não encontrado. Rode scripts/iniciar_linux.sh.' >&2; exit 1; }
[[ -f "$REPO/tools/build_ssao.sh" ]] || { echo 'tools/build_ssao.sh ausente.' >&2; exit 1; }
mkdir -p "$BASE/relatorios"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
LOG="$BASE/relatorios/build-${STAMP}.log"
echo "Log de build: $LOG"
(cd "$REPO" && bash tools/build_ssao.sh) 2>&1 | tee "$LOG"
python3 "$BASE/scripts/auditar_repositorio.py" "$REPO"
echo 'O build e a auditoria foram executados. Para avaliar visualmente, rode scripts/capturar_evidencias_linux.sh.'
