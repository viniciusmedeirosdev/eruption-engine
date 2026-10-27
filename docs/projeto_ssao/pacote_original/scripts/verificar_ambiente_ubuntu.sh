#!/usr/bin/env bash
set -u
printf '%s\n' '=== Verificação local (NÃO compila a engine) ==='
printf 'Sistema: '; uname -a
for cmd in git cmake g++ python3 curl vulkaninfo glslc spirv-val; do
  if command -v "$cmd" >/dev/null; then
    echo "[OK] $cmd: $(command -v "$cmd")"
  else
    echo "[FALTA] $cmd"
  fi
done
if command -v cmake >/dev/null; then cmake --version | head -1; fi
if command -v g++ >/dev/null; then g++ --version | head -1; fi
if command -v vulkaninfo >/dev/null; then
  echo '=== Vulkan detectado (não garante todas as features) ==='
  vulkaninfo --summary || echo '[ATENÇÃO] vulkaninfo falhou; verifique driver/display/ICD'
fi
if command -v grep >/dev/null && grep -qi 'avx2' /proc/cpuinfo 2>/dev/null; then
 echo '[OK] CPU informa AVX2'
else
 echo '[ATENÇÃO] AVX2 não confirmado: pode ser incompatível'
fi
printf '%s\n' 'Consulte documentacao/03_INSTALAR_EXECUTAR_UBUNTU.md para resolver dependências.'
