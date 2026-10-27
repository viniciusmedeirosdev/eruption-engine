# 3. Instalação, build e execução no Ubuntu 24.04

## 3.1 Plataforma testada e pré-requisitos

**Alvo primário documentado no fork:** Ubuntu 24.04 x86_64, CPU com AVX2, driver Vulkan 1.3 e as features solicitadas pelo motor. Internet é necessária para clonar dependências no CMake e obter o grande mapa de exemplo. Leia a documentação atual `eruption-engine/docs/ENTREGA_SSAO.md` após clonar, pois ela pode ter mudanças.

```bash
sudo apt-get update
sudo apt-get install -y git cmake ninja-build g++ python3 curl \
  libvulkan-dev glslc glslang-tools spirv-tools vulkan-tools \
  vulkan-validationlayers libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev zlib1g-dev
vulkaninfo --summary
```

**Verifique:** há um dispositivo Vulkan funcional, a engine pode criar a superfície, as features existem, o compilador conhece C++17, o CMake é adequado (README pede versão 3.28+), GPU/driver não está bloqueado e há espaço para mapa, assets e build. Em caso de múltiplas GPUs ou layers, anote driver e ICD escolhidos.

## 3.2 Clonar este fork (a partir da raiz do pacote)

```bash
bash scripts/iniciar_linux.sh
cd eruption-engine
git rev-parse HEAD
git status --short
```

O script configura `origin` para o fork e `upstream` para o projeto original, sem baixar/mesclar commits do upstream automaticamente. Se já existe um clone, o script não sobrescreve alterações locais.

## 3.3 Compilar e testar

```bash
bash tools/build_ssao.sh
```

O script usa `build-ssao`, build otimizado com símbolos `RelWithDebInfo`, habilita testes, verifica o mapa após o build e, se disponível, executa `spirv-val` para os shaders do SSAO/ambiente. Limita a paralelização a **dois jobs** por padrão para evitar esgotamento de RAM. Se tiver recursos: `BUILD_JOBS=4 bash tools/build_ssao.sh`.

Quando o mapa não for encontrado: `bash tools/fetch_demo_map.sh`; não marque build como aprovado com dependências ou cenário faltantes. O download tem checksum. Opcionalmente `cmake -S . -B build-ci -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DERUPTION_FETCH_DEMO_MAP=OFF -DERUPTION_BUILD_TESTS=ON` para apenas validar compilação sem baixar mapa; isso **não executa a demonstração**.

## 3.4 Abrir e comparar SSAO

```bash
bash tools/ssao_demo.sh on
bash tools/ssao_demo.sh off
SSAO_RADIUS=20 SSAO_STRENGTH=1.5 bash tools/ssao_demo.sh on
bash tools/ssao_demo.sh capture
```

Abra uma cena adequada, fixe ângulo de câmera, horário e configurações para comparar. `on/off` abre a engine; `capture` gera dentro de `evidencias/<timestamp>/` duas capturas reais `ssao-off.png` e `ssao-on.png`, logs e parâmetros. Quando executar interativamente, F2 ajusta SSAO e F3 mostra desempenho.

## 3.5 Modo de depuração e validação Vulkan

```bash
cmake -S . -B build-validation -DCMAKE_BUILD_TYPE=Debug -DERUPTION_FETCH_DEMO_MAP=ON
cmake --build build-validation --parallel 2
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation bash tools/ssao_demo.sh capture
```

Confira os logs (por exemplo `grep -RniE 'Validation Error|VUID-' evidencias/`). O código da engine grava o executável na raiz do checkout; builds distintos podem substituí-lo. Não compare FPS de binários Debug e RelWithDebInfo.

## 3.6 Testes adicionais obrigatórios antes de afirmar prontidão

- Fechar e reabrir; mudar on/off e raio sem travar; comparar mesma câmera.
- Redimensionar janela e repetir captura; observar semáforos, fences, layouts de imagem.
- Rodar no driver real da máquina de apresentação, com camadas de validação, se disponíveis.
- Conferir logs sem erros VUID; sem screenshots em branco; reproduzir resultados.
- Anotar commit, sistema, CPU/GPU, driver, versão Vulkan, presets e parâmetros das evidências.

Use também `bash ../scripts/testar_projeto_linux.sh` de dentro de `eruption-engine/` para gravar uma auditoria dos arquivos sem pressupor que ela substitua testes visuais.
