# 7. Diferenças entre especificação acadêmica e implementação

Os PDFs anexos servem como **documento histórico de projeto**. A engine escolhida na etapa final usa arquitetura própria; logo, não é correto afirmar equivalência literal.

| Item | Entregas 2/3 (proposta acadêmica) | Fork Eruption Engine (código existente) |
|---|---|---|
| Pipeline | 4 render passes RP0…RP3, subpass 0 | Vulkan 1.3 com **dynamic rendering** e pipeline deferido da engine |
| Kernel SSAO | **32 amostras** hemisféricas | **12 amostras** em disco/hemisfério orientado |
| Ruído | textura 4×4 de rotação | rotação pseudoaleatória por pixel (`ign`) |
| Buffer de AO | `R16F` proposto | `R8_UNORM` em meia resolução no fork |
| Blur bilateral | **5×5**, peso espacial/profundidade/normal | **3×3** guiado por profundidade |
| Reconstrução | pseudocódigo trabalha em view space / `invProj` | shader reconstrói em mundo usando `invViewProj` e depth |
| Composição | `lighting.frag` próprio e `FrameUBO` de 272 B | `ambient.frag`, `directional.frag`, `LightingUBO` do motor |
| Depth / normal | proposta convencional depth 0..1, clear=1; normal em view space | motor implementa depth com suas convenções, incluindo **reverse-Z**; normal codificada no G-buffer |
| Controle | parâmetros da especificação | interface F2 e variáveis `ERUPTION_TEST_SSAO_*` |

A compatibilidade conceitual é: G-buffer → SSAO → suavização edge-aware → iluminação → apresentação. As escolhas numéricas, espaço de coordenadas, formatos, buffers e sincronização diferem. Documente essas diferenças explicitamente no relatório/defesa: são adaptações motivadas por integrar uma engine existente, e não descumprimentos silenciosos.

## Revisão obrigatória antes da entrega

1. Compare o **enunciado específico da etapa final** com a documentação, código e evidências.
2. Verifique se há exigência acadêmica de implementações autorais ou um número mínimo de amostras; a adaptação de motor de terceiros deve ser apresentada como adaptação, com licença e referência.
3. Atualize os diagramas para refletir a engine real e não os quatro `VkRenderPass` tradicionais da proposta.
4. Evite afirmar que todos os shaders do PDF foram integrados tal qual. O fork usa seus próprios GLSL.
5. Se necessário evoluir para 32 amostras/blur 5x5, faça em uma branch e meça qualidade/performance; não altere parâmetros sem testes e sem atualizar C++/GLSL/documentação.
