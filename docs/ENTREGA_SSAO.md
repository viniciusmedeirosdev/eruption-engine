# Entrega final — SSAO em Vulkan

Este projeto utiliza um fork da Eruption Engine, em C++17 e Vulkan 1.3.
O SSAO e sua integração no renderer já existem no projeto original. As
adaptações deste fork são os comandos de demonstração/captura, a validação
automatizada, a correção do controle de raio e a detecção de falha dos pipelines.
Não se atribui ao grupo a autoria integral do motor ou do algoritmo original.

Base: https://github.com/eruptionlabs/eruption-engine
Commit de referência: `d356eadb4718a5382ff931505a7bbcb5c5d49c12`.
Licença do código: Apache 2.0; consulte também NOTICE e licenças dos assets.

## Plataforma e requisitos

O caminho documentado é **Ubuntu 24.04 x86_64**, com ambiente gráfico,
CPU com AVX2 e driver que exponha Vulkan 1.3 e os recursos solicitados pela
engine (incluindo descriptor indexing, dynamic rendering e synchronization2).
A engine contém código específico de Linux; estes scripts não são executáveis
Windows. No Windows, WSL só serve para a demonstração se `vulkaninfo` confirmar
os recursos necessários e houver apresentação gráfica funcional. A presença
de uma RTX 2080 Ti no computador, por si só, não comprova o suporte no WSL.

Instale as dependências no Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y git cmake ninja-build g++ python3 curl \
  libvulkan-dev glslc glslang-tools spirv-tools vulkan-tools \
  vulkan-validationlayers libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev zlib1g-dev
vulkaninfo --summary
```

Use o driver Vulkan do fabricante da GPU. O projeto baixa as dependências
C++ pelo CMake e o mapa `parana_field` pelo release original, verificando SHA-256.
É necessário acesso à internet na primeira compilação. Se o download do mapa
falhar, o script informa a falha; não considera apenas o build suficiente.

## Baixar, compilar e executar

```bash
git clone https://github.com/viniciusmedeirosdev/eruption-engine.git
cd eruption-engine
bash tools/build_ssao.sh
bash tools/ssao_demo.sh on
```

O build é incremental, usa dois processos por padrão para evitar esgotamento
de memória e executa os testes CPU registrados no CTest. Para mudar:
`BUILD_JOBS=4 bash tools/build_ssao.sh`.

Para comparar mantendo os mesmos argumentos de câmera e mapa:

```bash
bash tools/ssao_demo.sh off
bash tools/ssao_demo.sh on
SSAO_RADIUS=20 SSAO_STRENGTH=1.5 bash tools/ssao_demo.sh on
bash tools/ssao_demo.sh capture
```

`capture` executa duas vezes a engine real e salva PNGs, logs e parâmetros em
uma pasta nova dentro de `evidencias/`. Não gera imagens simuladas. O script
fixa o horário e desliga vento e profundidade de campo; outros efeitos dinâmicos
podem variar entre execuções, portanto o par não é um teste pixel a pixel.
Para uma comparação mais controlada, mantenha também câmera, clima e demais
configurações idênticos. Os argumentos adicionais são repassados à engine.

Na interface, F2 abre os controles de efeitos; na seção de iluminação é
possível habilitar SSAO e alterar intensidade e raio. O raio usa unidades de
mundo, não metros. F3 abre os indicadores de desempenho. Compare a mesma
vista com o efeito ligado/desligado, principalmente encontros de superfícies,
cantos e regiões próximas à base dos objetos.

## Pipeline implementado

1. Geometria → G-buffer (normal, profundidade e propriedades do material).
2. `ssao.frag` reconstrói a posição de mundo pela inversa de view-projection,
   orienta 12 amostras no hemisfério da normal e consulta a profundidade
   projetada. O resultado é armazenado em R8_UNORM, em meia resolução.
3. `ssao_blur.frag` aplica filtro bilateral 3×3, ponderado pela diferença de
   profundidade, para reduzir ruído preservando descontinuidades.
4. Iluminação amostra o resultado filtrado. No passe ambiente, o fator SSAO
   multiplica a contribuição ambiente; o motor também usa oclusão no passe
   direcional para seus termos de cavidade/iluminação indireta.
5. Pós-processamento → swapchain → apresentação.

Quando SSAO está desligado, as imagens são limpas com 1 (sem oclusão).
Os passes usam dynamic rendering, equivalente moderno aos objetos de render
pass/framebuffer. A implementação usa shaders vertex/fragment; compute não
é obrigatório para esta técnica.

## Mapeamento dos requisitos anteriores

| Requisito | Implementação |
|---|---|
| Instância, validação, superfície, seleção da GPU, dispositivo e filas | `src/renderer/VulkanContext.cpp` |
| Swapchain, image views, command pools e sincronização de frames | `src/renderer/VulkanContext.cpp` |
| G-buffer, imagens e transições | `src/renderer/GBuffer.cpp` |
| Pipelines SSAO, descriptors, UBO, passes e barreiras | `src/renderer/DeferredLighting.cpp` |
| Recursos e layout do LightingUBO | `src/renderer/DeferredLighting.hpp` |
| Cálculo SSAO | `shaders/lighting/ssao.frag` |
| Filtro bilateral | `shaders/lighting/ssao_blur.frag` |
| Composição | `shaders/lighting/ambient.frag`, `directional.frag` |
| Gravação de comandos e loop de renderização | `src/core/Render.cpp`, `Engine.cpp` |
| Controles interativos | `src/core/ImGui.cpp` |
| Liberação de recursos | Métodos `shutdown()` dos componentes |

O esquema concreto do fork usa reconstrução de posição a partir de depth.
Caso as entregas anteriores tenham proposto uma textura de posições ou outro
kernel, isso constitui uma diferença de implementação que deve ser explicada
na apresentação, não descrita como código idêntico ao pseudocódigo anterior.

## Validação e evidências

O workflow **SSAO build and validation** compila a engine, compila seus shaders,
executa os testes CPU existentes e valida SPIR-V dos passes SSAO e ambiente.
Ele publica um pacote Linux quando essas etapas passam. O mapa grande não
está nesse pacote; execute `bash tools/fetch_demo_map.sh` depois de extraí-lo.
O binário depende das bibliotecas de sistema do Ubuntu 24.04 e de driver Vulkan.

**Build aprovado não comprova renderização correta na GPU.** A validação final
inclui abrir o mapa na máquina de apresentação, conferir as duas capturas,
redimensionar a janela, alternar SSAO e encerrar sem erros de validação.
Para testar com validação Vulkan:

```bash
cmake -S . -B build-validation -DCMAKE_BUILD_TYPE=Debug -DERUPTION_FETCH_DEMO_MAP=ON
cmake --build build-validation --parallel 2
bash tools/ssao_demo.sh capture
```

A engine escreve o executável na raiz: o último build (Debug ou otimizado)
substitui esse executável. Para medir desempenho, recompile o alvo otimizado
ou use diretórios de checkout separados; não use os tempos de Debug.

Na entrega, inclua o link/commit do fork, código-fonte, instruções e evidências
reais produzidas na máquina utilizada. Não há screenshots ou FPS inventados.
O enunciado específico da última etapa ainda precisa ser confrontado com esta
implementação antes de afirmar conformidade integral.
