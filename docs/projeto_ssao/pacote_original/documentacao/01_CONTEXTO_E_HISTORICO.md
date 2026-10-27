# 1. Contexto, objetivo e evolução

## Objetivo acadêmico
Desenvolver e compreender uma aplicação de **Screen-Space Ambient Occlusion (SSAO)** integrada a uma arquitetura Vulkan, incluindo pesquisa bibliográfica, pipeline, shaders e integração C/C++ com a API, demonstrando o efeito e seu funcionamento.

**Equipe:** Felipe do Prado Marques Souza; Kenui Matheus Nascimento Ruas; Vinicius Medeiros Peres; Vitor Amorim Tartari. **Instituição:** Universidade Católica de Santos, Computação Gráfica, 2026.

## Entregas existentes

1. **Entrega 1 — revisão bibliográfica:** conceitos, equações, pesquisa e casos de aplicações/jogos. Referência no PDF conjunto `SSAO_Entregas_1_e_2.pdf` (inclui registros próprios dos jogos, conforme conteúdo disponível).
2. **Entrega 2 — arquitetura do pipeline Vulkan:** fluxo CPU → G-buffer → SSAO → bilateral → iluminação → apresentação; attachments, uniform/storage buffers, descriptors, sincronização, diagramas e justificativas. No mesmo PDF.
3. **Entrega 3 — pseudocódigo GLSL:** `geometry.vert`, `geometry.frag`, `fullscreen.vert`, `ssao.frag`, `bilateral.frag` e `lighting.frag`; no PDF `Entrega_3_Pseudocodigo_SSAO.pdf`. É **especificação acadêmica**, não fonte automaticamente executável da Eruption Engine.
4. **Entrega 4 — integração Vulkan C/C++:** no projeto existente, as funções de instância, device, swapchain, image views, pipeline/command buffers e sincronização pertencem à engine; arquivo central `src/renderer/VulkanContext.cpp`, com `Engine.cpp`, `Render.cpp`, `GBuffer.cpp` e `DeferredLighting.cpp`. Verifique os critérios exatos do professor antes da submissão.
5. **Entrega final — demo real em fork:** adaptar/validar Eruption Engine em Vulkan 1.3, controlar SSAO, tirar capturas reais ON/OFF, produzir logs e demonstrar execução. O arquivo `docs/ENTREGA_SSAO.md` no próprio fork é a referência técnica detalhada deste estágio.

## O que foi realmente desenvolvido no fork
A Eruption Engine já possuía SSAO; o fork **não é um motor criado do zero pelo grupo**. A documentação do fork atribui suas modificações a ferramentas de build/captura, correções Vulkan, controle do raio, validação e saneamento de carregamento/configurações. Evite alegar autoria integral ou que o SSAO foi implementado a partir de uma folha em branco.

O `main` inspecionado estava no commit `7c053e6b5220d98af1af1462a4748df75d26a0c8` e foi detectado como fork público. O repositório original continua evoluindo de modo independente; não sincronize o upstream sem revisar diferenças e recompilar.

## Limites do que este pacote comprova
Os PDFs originais recuperados integram a documentação acadêmica, e a aprovação histórica dos workflows demonstra um cenário automatizado específico. Este pacote **não foi compilado ou executado com Vulkan na máquina do usuário**; testes locais precisam ser produzidos e datados antes da entrega.
