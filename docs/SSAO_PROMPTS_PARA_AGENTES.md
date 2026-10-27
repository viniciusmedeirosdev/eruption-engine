# Prompts operacionais de continuidade (Codex, Cline ou outro agente)

Use um prompt por vez; os critérios de aceite e as ordens detalhadas estão em `AGENTS.md` e `docs/SSAO_ORDEM_DE_EXECUCAO.md`.

## Prompt mestre — iniciar agora

> Continue o meu projeto acadêmico de SSAO em Vulkan **neste checkout** da Eruption Engine; NÃO inicie outro repositório. Leia `AGENTS.md`, `docs/SSAO_ORDEM_DE_EXECUCAO.md`, `docs/SSAO_ESTADO_ATUAL.md`, `docs/ENTREGA_SSAO.md`, `docs/SSAO_CRITERIOS_DE_ACEITE.md` e o código referenciado. Execute a primeira ordem pendente, começando pela ORDEM 00: confira remotos, branch, SHA e arquivos locais e trabalhe em branch sem fazer reset nem commit/push em main. Depois mapeie o pipeline SSAO real, rode os testes que o ambiente permite e registre provas verdadeiras. Atualize `docs/SSAO_ESTADO_ATUAL.md` com resultados e próxima ação. Não presuma que as entregas anteriores e o código da engine sejam idênticos. Não invente prints, FPS nem testes. Ao terminar esta sessão, mostre exatamente arquivos editados, comandos, logs, êxitos, falhas e próximo passo.

## Fase 1 — auditoria Vulkan/C++

> Execute as ORDENS 00–01. Localize as funções de `VulkanContext`, `GBuffer`, `DeferredLighting`, `Engine`, `Render` e arquivos SSAO/blur/ambient/directional. Faça uma matriz requisito → arquivo/função → evidência no código → lacuna. Não implemente recursos especulativos; primeiro apresente o diagnóstico baseado no código atual.

## Fase 2 — build e evidências

> Execute as ORDENS 02–04 se a toolchain/driver permitirem. Instale dependências conforme documentação existente, execute o build com `pipefail`, CTest e spirv-val, verifique mapa e crie captura OFF/ON nativa. Registre GPU, driver, commit, parâmetros, logs e resultado de cada comando. Se não for possível abrir janela Vulkan, deixe a execução local pendente e documente o bloqueio sem fabricar evidência.

## Fase 3 — bugs e regressão

> Execute a ORDEM 05. Ative validation layers, teste recursos/semáforos/barreiras/descriptors, resize da swapchain e shutdown. Faça correções mínimas em branch, explique causa, registre VUID (se houver) e execute novamente build, CTest, spirv-val e renderizações ON/OFF. Não use `git reset --hard`, `git clean`, force push ou merge cego.

## Fase 4 — aderência acadêmica e finalização

> Execute a ORDEM 07, e ORDEM 06 apenas se o enunciado realmente exigir ou se um bug justificar. Compare Entregas 1–3 com o código atual: kernel 32→12, blur 5×5→3×3, render passes clássicos→dynamic rendering, UBOs e depth diferentes. Faça relatório fiel, com tabela de requisitos, diagrama da engine, README e autoria/Apache LICENSE+NOTICE. Não alegue "100% validado" sem executar build, runtime, captura e verificação de todos os itens do professor.

## Prompt de retomada (qualquer nova conversa)

> Retome o SSAO/Vulkan a partir de `docs/SSAO_ESTADO_ATUAL.md`. Leia `AGENTS.md`, confira o Git real e revalide o que tiver mudado. Não repita trabalho já registrado. Continue na primeira ORDEM pendente, atualize o handoff com comandos e resultados reais e pare apenas em ponto coerente de revisão/commit.
