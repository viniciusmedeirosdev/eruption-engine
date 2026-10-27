#!/usr/bin/env bash
set -Eeuo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="$BASE/eruption-engine"
FORK='https://github.com/viniciusmedeirosdev/eruption-engine.git'
UPSTREAM='https://github.com/eruptionlabs/eruption-engine.git'
CLONE_FRESH=0
command -v git >/dev/null || { echo 'Erro: instale git.' >&2; exit 1; }
if [[ -e "$TARGET" && ! -d "$TARGET/.git" ]]; then
  echo "Erro: $TARGET existe, mas não é um checkout Git; nada foi alterado." >&2
  exit 1
fi
if [[ ! -d "$TARGET/.git" ]]; then
  echo 'Clonando o fork e os submódulos (requer conexão GitHub)...'
  git clone --recurse-submodules "$FORK" "$TARGET"
  CLONE_FRESH=1
else
  echo 'Checkout já existe. Não farei pull, checkout ou sobrescrita automática.'
fi
cd "$TARGET"
if ! git remote get-url upstream >/dev/null 2>&1; then
  git remote add upstream "$UPSTREAM"
fi
printf '\nCommit: '; git rev-parse HEAD
printf '\nRemotos:\n'; git remote -v
printf '\nStatus (vazio = limpo):\n'; git status --short
printf '\nSubmódulos:\n'; git submodule status || true
if [[ "$CLONE_FRESH" == 1 ]]; then
  echo 'Adicionando orientacoes de continuidade ao checkout novo (sem sobrescrita)...'
  python3 "$BASE/scripts/instalar_ordens_no_fork.py" "$TARGET" || echo 'Aviso: consulte o instalador de ordens; a clonagem foi concluida.'
else
  echo 'Checkout já existente: para adicionar ordens sem sobrescrever, rode python3 scripts/instalar_ordens_no_fork.py na raiz do pacote.'
fi
printf '\nPróximo passo: leia documentacao/03_INSTALAR_EXECUTAR_UBUNTU.md no pacote; execute, daqui, bash tools/build_ssao.sh\n'
