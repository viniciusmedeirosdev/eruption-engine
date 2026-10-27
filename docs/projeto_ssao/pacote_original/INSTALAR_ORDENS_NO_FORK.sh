#!/usr/bin/env bash
set -euo pipefail
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec python3 "$BASE/scripts/instalar_ordens_no_fork.py" "$@"
