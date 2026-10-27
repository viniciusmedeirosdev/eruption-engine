# 8. Plano de continuidade, em ordem de execução

## Fase 0 — Preparar checkout seguro

- Abrir `LEIA_PRIMEIRO.md`; extrair tudo mantendo a árvore original.
- Executar um dos scripts `iniciar_*`. Confirmar `git remote -v`, `git rev-parse HEAD`, `git status`.
- Criar uma branch para alterações (`git switch -c trabalho-ssao-2026` se ainda não existir); nunca mexer no código original diretamente sem controle de versão.
- Ler `eruption-engine/docs/ENTREGA_SSAO.md`, `README.pt-BR.md`, `LICENSE` e `NOTICE`.

## Fase 1 — Preparar ambiente e fazer build confiável

- Instalar pacotes, confirmar `vulkaninfo --summary` e a capacidade real do device.
- Executar `bash tools/build_ssao.sh`, salvar stdout/stderr num log e checar código de retorno.
- Verificar se todos os shaders/SPIR-V e testes CTest passam; confirmar mapa GLB.
- Se falhar, diferenciar falta de dependências/rede, recurso Vulkan ausente e bugs no código.

## Fase 2 — Executar SSAO como está (baseline)

- Criar capturas OFF e ON nativas (`tools/ssao_demo.sh capture`), guardando PNGs e logs.
- Anotar versão do driver, dispositivo, commit, preset, resolução, raio e força.
- Abrir `F2` e `F3` se a engine suportar interação nesse ambiente, testar controls.
- Rodar Debug/validation layer e gravar ausência/presença de VUID.

## Fase 3 — Auditoria funcional de Vulkan

- Instância/extensões/layers; seleção de GPU/queue families; logical device; superfície/swapchain.
- Layouts e image views; dynamic rendering; descriptors e pipelines; command pools/buffers.
- Sincronização (barreiras entre SSAO raw/blur/lighting, frames em voo e fences).
- Limpeza de recursos ao terminar; reconstrução da swapchain ao resize.

## Fase 4 — Alinhar relatório à implementação

- Atualizar desenho técnico da arquitetura para **engine real** (ver `07_DIFERENCAS...`).
- Explicar 12 amostras e blur 3x3, em oposição ao pseudocódigo 32/5x5.
- Separar autoria do grupo versus código de terceiros e incluir link/commit/licença.
- Produzir screenshots da sua execução e análise objetiva ON/OFF.

## Fase 5 — Evoluções opcionais (só após baseline)

1. Comparativo entre 12/24/32 amostras, com controle por configuração.
2. Blur alternativo 5x5 e comparação de vazamento em arestas.
3. Otimização de qualidade/performance, incluindo meia resolução versus resolução inteira.
4. Testes de regressão de janela/resize e de recursos Vulkan.

**Regra de conclusão:** build e testes automatizados **mais** execução real **mais** evidências ON/OFF **mais** conformidade com enunciado **mais** documentação de autoria. Sem cumprir esses itens, não declarar "100% funcional".


## Continuidade operacional no Codex

O roteiro detalhado, com as ORDENS 00–07, prompts por fase, critérios de aceite e registro vivo da sessão está em `ordens_para_fork/docs/`. Instale-o no checkout via `INSTALAR_ORDENS_NO_FORK.bat` ou `bash INSTALAR_ORDENS_NO_FORK.sh` e siga `eruption-engine/AGENTS.md`. Este é o fluxo preferido para continuar o trabalho com agente de IA.
