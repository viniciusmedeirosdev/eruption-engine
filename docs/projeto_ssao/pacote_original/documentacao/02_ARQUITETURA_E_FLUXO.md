# 2. Arquitetura SSAO — implementação do fork

## Arquitetura real: ordem de passes

```text
Cena / geometria / buffers C++
       |
       v
VulkanContext: instance + surface + physical/logical device + queues
       |
       v
Engine/Render: acquire swapchain image, command buffer e frames em voo
       |
       v
GBuffer: geometry, normais codificadas, depth, PBR/albedo/emissive
       |
       v
DeferredLighting: SSAO meia resolução (shaders/lighting/ssao.frag)
       |         12 amostras hemisféricas orientadas por normal
       |         reconstrução da posição usando depth + invViewProj
       v
SSAO raw R8_UNORM -> filtro bilateral 3 x 3 (ssao_blur.frag)
       |
       v
Iluminação ambiente / direcional usando AO filtrada
       |
       v
Postprocess / apresentação via swapchain
```

O motor utiliza **Vulkan 1.3 e dynamic rendering**, substituindo o render pass/framebuffer clássico quando cabível; as transições e barreiras de imagens continuam obrigatórias. A documentação do fork lista `dynamic_rendering`, `synchronization2`, descriptor indexing e `independentBlend` como requisitos do dispositivo. C++17 é a linguagem de base.

## Detalhamento dos dados

- `gDepth` e `gNormal` são texturas amostradas pelo SSAO. A normal é decodificada do G-buffer antes de orientar o kernel.
- `LightingUBO` (bindings previstos nos GLSL da engine) carrega luzes, configuração do SSAO, matriz `viewProj` e `invViewProj`; manter alinhamento/offset do C++ e GLSL é requisito de segurança.
- `ssao.frag`: obtém posição mundial com `invViewProj * vec4(uv*2-1, d, 1)`; constrói base tangente/bitangente; roda 12 direções em disco com rotação pseudoaleatória por pixel; reprojeta amostras na tela, compara superfícies, aplica alcance/viés e grava acessibilidade 0..1.
- `ssao_blur.frag`: amostra 3x3 sobre a textura de meia resolução e calcula pesos com diferença de profundidade. A saída preserva bordas melhor que uma média simples.
- `ambient.frag` (e termos de `directional.frag`): aplica AO filtrada à iluminação apropriada, sem substituir as outras etapas do material e da iluminação.
- Com SSAO OFF o código da engine limpa as imagens com 1, indicando nenhum escurecimento adicional.
- O controle de interface fica em F2 (efeitos/iluminação); F3 mostra indicadores de desempenho. O raio do SSAO usa **unidades de mundo**, não uma escala fixa em metros.

## Vulkan da entrega 4: localizações e verificação

| Requisito | Onde procurar e o que verificar |
|---|---|
| Instância, extensões, validação | `src/renderer/VulkanContext.cpp`; camadas e debug utils no modo Debug |
| Surface, GPU, device, queues | `VulkanContext.cpp`; requisitos de features e família de filas |
| Swapchain e views | `VulkanContext.cpp`; reconstrução ao redimensionar |
| Render pass/framebuffers equivalentes | `GBuffer.cpp`, `DeferredLighting.cpp`; **dynamic rendering** |
| Pipelines, layouts, descriptors | `DeferredLighting.cpp/.hpp`, `PipelineBuilder.cpp` |
| Vertex/index/UBO/GPU buffers | `ModelRenderer.cpp`, `DeferredLighting.hpp` e `GBuffer.cpp` |
| Command pools/buffers | `VulkanContext.cpp`, `src/core/Render.cpp` |
| Semáforos/fences/barreiras | `VulkanContext.cpp`, `DeferredLighting.cpp` |
| Loop acquire→submit→present | `src/core/Engine.cpp`, `Render.cpp` |
| Cleanup RAII/shutdown | métodos `shutdown()` e destructors nos componentes |

## Riscos de bugs específicos

1. **Recursos por frame:** não reutilizar/destruir descriptors, uniform buffers ou imagens antes da GPU liberar o frame associado à fence.
2. **Resize/swapchain:** recriar targets/depth/SSAO nas dimensões corretas; não reter views antigas.
3. **Barreiras:** produzir SSAO somente depois da disponibilidade de depth/normal; filtrar somente depois do SSAO raw; iluminação somente depois do AO filtrado.
4. **Layouts e bindings:** conferir a mesma definição de `LightingUBO` em C++ e shaders; atualizar pipeline layout quando alterar bindings.
5. **Qualidade visual:** medir uma cena com SSAO ON/OFF na mesma câmera, horário, exposição e preset, evitando atribuir diferenças a nuvens, sombras ou DoF.
6. **Performance:** observar escala de renderização, número de amostras e resolução; não equiparar performance de software Vulkan a GPU real.

Código original e decisões técnicas pertencem ao fork citado acima; esta seção é um mapa de manutenção, não reprodução integral do código do motor.
