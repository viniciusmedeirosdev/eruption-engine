# Vulcão SSAO

Demonstração de **oclusão de ambiente em espaço de tela (SSAO)** em C++20 e Vulkan 1.3:
uma câmera livre voa pelo mapa do vulcão enquanto uma janela de controles altera os
parâmetros dos shaders em tempo real.

Este repositório é uma versão reduzida, feita para apresentação, do fork da
[Eruption Engine](https://github.com/eruptionlabs/eruption-engine). O motor completo
(cerca de 80 mil linhas) foi substituído por **um único arquivo C++ e seis shaders**,
todos comentados. O mapa e o algoritmo de SSAO vêm da engine original (ver `NOTICE`);
o motor completo continua disponível no histórico do Git e no repositório original.

## Arquivos

| Arquivo | O que faz |
|---|---|
| `main.cpp` | O programa inteiro: Vulkan, carregamento do mapa, câmera, interface e os 4 passos de desenho |
| `shaders/comum.glsl` | Parâmetros compartilhados entre o C++ e todos os shaders |
| `shaders/gbuffer.vert` / `.frag` | Passo 1: desenha o mapa guardando cor, normal e profundidade (G-buffer) |
| `shaders/tela.vert` | Triângulo de tela cheia usado pelos passos 2, 3 e 4 |
| `shaders/ssao.frag` | Passo 2: calcula a oclusão de ambiente |
| `shaders/blur.frag` | Passo 3: suaviza o ruído da oclusão (blur bilateral) |
| `shaders/final.frag` | Passo 4: ilumina a cena usando a oclusão |
| `CMakeLists.txt` | Compila tudo, baixa as bibliotecas e o mapa |

## Compilar e executar (Ubuntu 24.04)

```bash
sudo apt-get install -y git cmake ninja-build g++ libvulkan-dev glslc libglfw3-dev libglm-dev
cmake -S . -B build -G Ninja
cmake --build build
./build/vulcao
```

É preciso uma GPU com driver Vulkan 1.3 e internet na primeira configuração: o CMake
baixa Dear ImGui, cgltf, stb e o mapa do vulcão (~170 MB, conferido por SHA-256). O
carregamento do mapa leva alguns segundos (163 texturas). Para compilar com a camada
de validação do Vulkan (pacote `vulkan-validationlayers`), use
`cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`.

## Controles

| Entrada | Ação |
|---|---|
| Botão direito do mouse (segurar) + mover | Olhar ao redor |
| `W` `A` `S` `D` | Andar |
| `Espaço` / `Ctrl` | Subir / descer |
| `Shift` | Acelerar 5x |
| `Esc` | Sair |

Janela de controles:

| Controle | Efeito no shader |
|---|---|
| SSAO ligado | Aplica ou não a oclusão na imagem final |
| Raio | Alcance da busca por oclusão, em unidades de mundo |
| Intensidade | Força do escurecimento |
| Contraste | Expoente aplicado ao resultado |
| Bias | Tolerância que evita a superfície sombrear a si mesma |
| Amostras | Pontos testados por pixel (qualidade x custo) |
| Suavizar (blur) | Desligado, mostra o ruído bruto do SSAO |
| Modo | Final, somente oclusão, argila (sem texturas), normais, profundidade |
| Divisória | Linha vertical: à esquerda sem SSAO, à direita com SSAO |
| Luz ambiente / Luz do sol | Intensidade de cada luz (a oclusão atua na ambiente) |
| AO na luz direta | Faz a oclusão escurecer também a luz do sol, para exagerar o efeito |
| Velocidade | Velocidade da câmera |
| Alcance dos objetos | Distância máxima de desenho de plantas e pedras (menor = mais rápido) |

Sugestão de roteiro: modo **Somente oclusão** para mostrar o que o SSAO calcula;
desligar **Suavizar** para mostrar o ruído; variar **Raio** e **Amostras**; voltar ao
modo **Final** e arrastar a **Divisória** para comparar com e sem o efeito.

## Como o SSAO funciona aqui

1. **G-buffer.** O mapa é desenhado uma vez. Em vez de iluminar, cada pixel guarda
   cor, normal e profundidade em imagens.
2. **SSAO.** Para cada pixel, a posição no mundo é reconstruída a partir da
   profundidade. O shader testa N pontos em espiral no hemisfério da normal, projeta
   cada um na tela e lê a superfície real naquele ponto. Vizinhos acima do plano do
   pixel e dentro do raio contam como oclusão.
3. **Blur.** Média 5x5 que só mistura pixels de profundidade parecida, para a
   oclusão não vazar entre objetos.
4. **Final.** `cor = albedo × (ambiente × oclusão + sol)`, mais o brilho da lava e a
   interface.

## Onde está cada parte do Vulkan

| Requisito | Função em `main.cpp` |
|---|---|
| Instância, validação, superfície, GPU, dispositivo, fila, sincronização | `iniciarVulkan` |
| Swapchain, image views e imagens do G-buffer/oclusão | `criarTela` |
| Buffers de vértices/índices e texturas com mipmaps | `carregarMapa`, `criarTextura`, `enviarParaGPU` |
| Descritores e buffer de parâmetros (UBO) | `criarDescritores`, `ligarImagensDaTela` |
| Pipelines (dynamic rendering) | `criarPipeline`, `criarPipelines` |
| Gravação de comandos, transições de layout e apresentação | `desenharQuadro`, `transicao` |
| Liberação de recursos | filas `limpezaGeral` / `limpezaTela` e fim de `main` |

## O que foi verificado

Compilado e executado em Ubuntu 24.04 com Vulkan por software (Mesa llvmpipe), com a
camada de validação ativa e sem mensagens de erro: carregamento do mapa, os modos de
visualização, a divisória, movimento e giro da câmera, redimensionamento da janela e
encerramento. Não foi testado em GPU física nem em Windows; o desempenho em tempo
real depende da placa (reduza **Alcance dos objetos** se ficar lento).

## Licença

Código sob [Apache License 2.0](LICENSE). Créditos e bibliotecas em [NOTICE](NOTICE).
"Eruption Engine" é marca da Gdg Soluções Digitais LTDA.
