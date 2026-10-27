# 5. Mapa de arquivos reais do fork (GitHub)

A árvore principal do fork, verificada em 27/10/2026, inclui diretórios `src/`, `shaders/`, `assets/`, `data/`, `tools/`, `tests/`, `third_party/`, `cmake/`, `.github/workflows/` e `docs/`. Para listar **todos os caminhos** do checkout, execute `python3 ../scripts/auditar_repositorio.py eruption-engine` a partir da raiz do pacote ou `git ls-files` dentro do clone.

| Caminho no clone | Função no trabalho |
|---|---|
| `README.md`, `README.pt-BR.md` | visão geral/requisitos da engine |
| `docs/ENTREGA_SSAO.md` | principal guia de entrega e validação do fork |
| `CMakeLists.txt` | configuração/compilação de engine, shaders, targets e testes |
| `src/main.cpp` | entry point |
| `src/renderer/VulkanContext.cpp/.hpp` | Vulkan instance, layers, GPU, device, queues, presentation, frame sync |
| `src/renderer/GBuffer.cpp/.hpp` | criação/uso do G-buffer |
| `src/renderer/DeferredLighting.cpp/.hpp` | SSAO e iluminação deferida, pipeline, descriptors e recursos |
| `src/renderer/PipelineBuilder.cpp/.hpp` | configuração/compilação do pipeline gráfico |
| `src/core/Engine.cpp/.hpp`, `src/core/Render.cpp` | loop, submissões e fluxo de frame |
| `src/core/ImGui.cpp` | interface de controles gráficos |
| `shaders/lighting/ssao.frag` | cálculo SSAO do motor; 12 amostras, meia resolução |
| `shaders/lighting/ssao_blur.frag` | blur bilateral 3x3 guiado por profundidade |
| `shaders/lighting/ambient.frag` | composição ambiente com AO |
| `shaders/lighting/directional.frag` | luz direcional e termos de cavidade/indireta |
| `shaders/gbuffer/*` | geometria, superfícies, depth e normal |
| `shaders/postprocess/*` | pós-processamento para apresentação |
| `data/graphics.json` | presets low/medium/high; low e medium podem deixar SSAO OFF |
| `data/postprocess.json` | exposição, DoF, bloom e outros efeitos confundidores |
| `assets/external/parana_field/*` | mapa de exemplo; GLB pesado obtido de release separado |
| `tools/build_ssao.sh` | build incremental, CTest, SPIR-V e requisito mapa |
| `tools/ssao_demo.sh` | liga/desliga/captura com parâmetros fixos |
| `tools/fetch_demo_map.sh` | baixa mapa e valida checksum |
| `.github/workflows/ssao-build.yml` | build+testes+pacote Linux |
| `.github/workflows/ssao-runtime.yml` | renderização com software Vulkan e upload de evidências |
| `.gitmodules` | submódulo `tools/caldera` |
| `LICENSE`, `NOTICE` | licenças e avisos legais a preservar |

## O que NÃO é parte do código do fork

- `documentacao/entregas_academicas/*.pdf`: artefatos acadêmicos originais recuperados para continuidade.
- `documentacao/*.md`, `scripts/*.sh`, `scripts/*.ps1`: material deste pacote de orientação, não source tree do motor.
- Pseudocódigos da Entrega 3 não são os shaders compilados do fork; evite sobrescrever `shaders/lighting/*.frag` pelos textos do PDF.

## Mudanças de estrutura

Se arquivos do GitHub tiverem sido movidos após o commit de referência, use `git log --follow -- <arquivo>` e `git grep -n 'SSAO'`, `git grep -n 'ssaoParams'`, `git grep -n 'vkCmdPipelineBarrier2'` para encontrar as classes atualizadas; reveja também paths dos scripts.
