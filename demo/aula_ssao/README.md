# Demonstração de SSAO para a aula

Demonstração da oclusão de ambiente em espaço de tela (SSAO) do fork da
Eruption Engine, preparada para apresentação em sala (Computação Gráfica, UNISANTOS).

## Imagens (capturas reais da engine)

Todas foram geradas pela própria engine no mapa `parana_field`, na mesma execução
e com a mesma câmera, em uma NVIDIA GeForce RTX 2080 Ti (Windows 10, driver 616.92,
Vulkan 1.4), resolução 1920×1061, raio 12 u, intensidade 0,8.

| Arquivo | Conteúdo |
|---|---|
| `imagens/1_cena_ssao_desligado.png` | Cena final com SSAO desligado |
| `imagens/2_cena_ssao_ligado.png` | Mesma cena com SSAO ligado (12 amostras, blur bilateral 3×3) |
| `imagens/3_buffer_ssao_12_amostras.png` | Só o buffer de oclusão (branco = sem oclusão), kernel padrão da engine |
| `imagens/4_buffer_ssao_32_amostras.png` | Só o buffer de oclusão, kernel da proposta acadêmica (32 amostras, blur 5×5) |
| `imagens/5_zoom_12_vs_32_amostras.png` | Ampliação: 12 amostras (esq.) × 32 amostras (dir.) |
| `imagens/6_zoom_desligado_vs_ligado_clareado.png` | Ampliação clareada ×2,5: desligado (esq.) × ligado (dir.) |

As imagens 5 e 6 são recortes ampliados das capturas 1–4 (a 6 também teve o
brilho aumentado para facilitar a leitura no projetor); não são renderizações novas.

Tempo de GPU da iluminação diferida (inclui os passes SSAO e blur), média de
100 quadros na mesma cena: **0,93 ms** desligado, **1,10 ms** com 12 amostras/3×3,
**1,28 ms** com 32 amostras/5×5.

## Demonstração ao vivo (Windows)

Requer o executável `eruption-engine.exe` compilado na raiz do repositório (build
MSYS2 UCRT64) e o mapa `assets/external/parana_field/parana_field.glb`.
Os lançadores procuram as DLLs do MSYS2 em `D:\msys64\ucrt64\bin`; se estiverem em
outro lugar, defina a variável `MSYS2_BIN` antes.

| Lançador | O que mostra |
|---|---|
| `1_ABRIR_ENGINE_SSAO_LIGADO.bat` | Engine com SSAO ligado (parâmetros padrão) |
| `2_ABRIR_ENGINE_SSAO_FORTE.bat` | SSAO exagerado (intensidade 2,0, raio 20), bem visível no projetor |
| `3_ABRIR_ENGINE_SO_BUFFER_SSAO.bat` | Somente o buffer de oclusão |

Na engine: **F2** abre o painel de efeitos — na seção de iluminação é possível
ligar/desligar o SSAO, alterar raio e intensidade e ativar o kernel acadêmico
(32 amostras, blur 5×5). **F3** mostra FPS e tempos de GPU.

## Como o efeito é calculado (resumo)

1. O G-buffer grava profundidade e normal de cada pixel.
2. `shaders/lighting/ssao.frag` (meia resolução) reconstrói a posição de mundo a
   partir da profundidade, distribui amostras no hemisfério orientado pela normal
   e conta quantas ficam atrás da geometria visível.
3. `shaders/lighting/ssao_blur.frag` aplica filtro bilateral ponderado por
   profundidade (remove ruído sem borrar bordas).
4. Os passes de iluminação (`ambient.frag`, `directional.frag`) multiplicam a
   luz ambiente/indireta pelo fator de oclusão.
