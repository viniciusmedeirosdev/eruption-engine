# AGENTS.md — SSAO Vulkan / Eruption Engine (fork)

## Missão

Continuar o **repositório existente**, não começar uma engine nova. Objetivo: entregar demonstração verificável de SSAO usando Vulkan, com código C++17/GLSL integrado, documentação de arquitetura, validação e evidências autênticas para a UNISANTOS.

Fork: https://github.com/viniciusmedeirosdev/eruption-engine  
Origem: https://github.com/eruptionlabs/eruption-engine  
Snapshot de referência do pacote (27/10/2026): `7c053e6b5220d98af1af1462a4748df75d26a0c8` (confirmar HEAD no início de cada sessão).

## Ordem obrigatória de leitura

1. `docs/SSAO_ORDEM_DE_EXECUCAO.md` — plano que deve ser seguido em sequência.
2. `docs/SSAO_ESTADO_ATUAL.md` — registro de progresso e impedimentos; atualizar durante a execução.
3. `docs/SSAO_CRITERIOS_DE_ACEITE.md` — critérios para chamar etapa de concluída.
4. `docs/SSAO_PROMPTS_PARA_AGENTES.md` — comandos de trabalho por fase.
5. `docs/ENTREGA_SSAO.md`, `README.pt-BR.md`, `LICENSE`, `NOTICE` e workflows CI.
6. Código de referência: `src/renderer/VulkanContext.cpp`, `src/renderer/GBuffer.cpp`, `src/renderer/DeferredLighting.cpp`, `src/renderer/DeferredLighting.hpp`, `src/core/Render.cpp`, `src/core/Engine.cpp`, `shaders/lighting/ssao.frag`, `shaders/lighting/ssao_blur.frag`, `shaders/lighting/ambient.frag`, `shaders/lighting/directional.frag`.

## Ordens de trabalho

- 01. **Diagnosticar primeiro**: `git status`, remotos, HEAD, submódulos, alterações locais, distro, compilador, Vulkan SDK/driver e GPU. Não substituir nem limpar trabalhos não rastreados.
- 02. **Fazer baseline**: compilar C++/GLSL, executar CTest/spirv-val, registrar códigos de saída; baixar assets legítimos. Se não houver hardware/driver Vulkan, anotar impossibilidade de testes de execução.
- 03. **Testar o motor existente**: captura de SSAO ON/OFF na mesma cena e parâmetros; logs reais, Vulkan validation layers, comportamento de resize e encerramento. Não fabricar captura visual nem métricas.
- 04. **Auditar requisitos Vulkan**: instância, layers/extensões, seleção physical/logical device, filas, surface, swapchain/images, dynamic rendering, descriptors, buffers, pipelines, comando/sincronização e cleanup. Apontar arquivos e funções.
- 05. **Corrigir somente defeitos reproduzidos**, um por vez em branch, com teste de regressão. Não migrar cegamente código do pseudocódigo acadêmico.
- 06. **Alinhar documentação**: a implementação da Eruption Engine usa 12 amostras e filtro 3×3 de meia resolução; as entregas acadêmicas propõem 32 amostras e filtro 5×5. Não dizer que são idênticas. Verificar exato enunciado antes de afirmar conformidade.
- 07. **Registrar entrega**: documentação, evidências, commits, passos de reprodução, autoria/proveniência, limitações e checklist. Apresentar testes que realmente ocorreram.

## Segurança do repositório

Trabalhar em branch `trabalho-ssao-continuacao` (ou branch de trabalho existente). Não commitar dependências/binários/GLB/cache, segredos ou evidências volumosas sem analisar. Não executar `git reset --hard`, `git clean -fd`, `git push --force`, merge/rebase de upstream ou deletar trabalho do usuário automaticamente. Não criar falso resultado de teste.

## Resposta e handoff em cada sessão

Informar: (1) commit/branch e estado do working tree; (2) arquivos realmente editados; (3) comandos executados e resultado; (4) evidências/logs com caminhos verdadeiros; (5) problemas em aberto e **próxima ordem numerada**. Atualizar `docs/SSAO_ESTADO_ATUAL.md` sem assinalar etapas não testadas como concluídas.
