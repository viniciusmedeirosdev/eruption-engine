#!/usr/bin/env bash
set -Eeuo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO="$BASE/eruption-engine"
[[ -x "$REPO/eruption-engine" ]] || { echo 'Binário ausente. Execute scripts/testar_projeto_linux.sh.' >&2; exit 1; }
[[ -f "$REPO/tools/ssao_demo.sh" ]] || { echo 'Script de captura não encontrado.' >&2; exit 1; }
cd "$REPO"
# Passa quaisquer opções extras diretamente para tools/ssao_demo.sh.
bash tools/ssao_demo.sh capture "$@"
echo 'Confira evidencias/*/ssao-off.png, ssao-on.png, logs e parametros.txt'
echo 'Se usar validation layer, confira logs e anote driver, GPU e commit.'
