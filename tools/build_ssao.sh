#!/usr/bin/env bash
# Build incremental da demonstracao; executa a partir de qualquer diretorio.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build-ssao"
for tool in cmake g++ python3; do
    command -v "$tool" >/dev/null || { echo "Falta $tool. Consulte docs/ENTREGA_SSAO.md." >&2; exit 1; }
done
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DERUPTION_BUILD_TESTS=ON -DERUPTION_FETCH_DEMO_MAP=ON
cmake --build "$BUILD_DIR" --parallel "${BUILD_JOBS:-2}"
ctest --test-dir "$BUILD_DIR" --output-on-failure
if command -v spirv-val >/dev/null; then
    for shader in ssao.frag ssao_blur.frag ambient.frag; do
        spirv-val --target-env vulkan1.3 "$ROOT/shaders/lighting/$shader.spv"
    done
fi
[[ -s "$ROOT/assets/external/parana_field/parana_field.glb" ]] || {
    echo "O build terminou, mas falta o mapa. Execute bash tools/fetch_demo_map.sh." >&2
    exit 1
}
echo "Build concluido. Execute: bash tools/ssao_demo.sh on"
