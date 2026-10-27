# 11. Prompt pronto para Codex / agente de código

Copie o texto abaixo para o agente que terá acesso local ao checkout ou à integração GitHub.

---

Você é engenheiro sênior de computação gráfica C++/Vulkan. Trabalhe **no meu fork já existente** `https://github.com/viniciusmedeirosdev/eruption-engine`, sem iniciar um motor do zero. Objetivo: dar continuidade ao projeto acadêmico SSAO/Vulkan da UNISANTOS, preparar a entrega final com código demonstrável, documentação coerente e evidências reais. Antes de editar, inspecione `docs/ENTREGA_SSAO.md`, `README.pt-BR.md`, `CMakeLists.txt`, `.github/workflows/ssao-build.yml`, `.github/workflows/ssao-runtime.yml`, `src/renderer/VulkanContext.cpp`, `GBuffer.cpp`, `DeferredLighting.cpp/.hpp`, `src/core/Render.cpp`, `Engine.cpp`, `shaders/lighting/ssao.frag`, `ssao_blur.frag`, `ambient.frag`, `directional.frag` e scripts `tools/build_ssao.sh`, `tools/ssao_demo.sh`.

Referência de estado inspecionado: `main` em `7c053e6b5220d98af1af1462a4748df75d26a0c8` (27/10/2026). Não faça merge automático do upstream e não quebre main; desenvolva numa branch e proponha PR.

Primeiro: (1) liste a estrutura real; (2) identifique ambiente, toolchain, Vulkan device e features; (3) compile C++ e GLSL, rode CTest/spirv-val; (4) execute SSAO OFF e ON no mapa, com screenshots **da aplicação real**; (5) execute com validation layers, examine VUIDs; (6) valide swapchain resize e shutdown; (7) compare efeitos/qualidade e documente limitação de hardware/CI.

Os PDFs acadêmicos anteriores propõem 32 amostras, blur bilateral 5x5, 4 passes e FrameUBO em view space. O código da Eruption Engine usa 12 amostras, blur 3x3, meia resolução R8 e dynamic rendering com UBO/GLSL da engine. Não afirme equivalência literal: documente diferenças; altere a técnica somente se houver requisito ou ganho demonstrado. A origem do SSAO é da engine original; cite licenças/autoria e descreva contribuições do fork.

Entregue: alterações source controladas, scripts reproduzíveis, README técnico, tabela requisito→arquivo/função, relatório de testes com comandos/códigos de saída, capturas reais, logs, commit SHA e pendências. Não invente screenshots, métricas, testes ou promessas de 100% de correção. Se o ambiente não conseguir executar Vulkan ou acessar assets, registre o impedimento e faça o máximo de análise estática/testes disponíveis, explicando exatamente o que não foi verificado. Inclua checklist contra o enunciado específico da etapa final.

---
