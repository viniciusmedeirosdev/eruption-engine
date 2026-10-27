# Ordens de continuidade — leitura rápida

Este arquivo serve para dar continuidade ao trabalho quando o projeto for aberto por outro agente ou colaborador. Para instruções completas, leia `CONTINUAR_AQUI.md` e, depois do clone, `eruption-engine/AGENTS.md` e `eruption-engine/docs/SSAO_ORDEM_DE_EXECUCAO.md`.

## Sequência obrigatória

1. **Preservar projeto:** identificar origin/upstream, branch, commit, arquivos locais e estado dos submódulos; não sobrescrever alterações.
2. **Mapear implementação real:** percorrer VulkanContext, GBuffer, DeferredLighting, Render/Engine e shaders SSAO/blur/ambient, identificando o papel de cada arquivo.
3. **Conferir o ambiente:** verificar Ubuntu/Vulkan 1.3, device/driver, CMake/C++ e GLSL, baixar mapa de demonstração com hash legítimo.
4. **Criar baseline:** executar build, CTest e spirv-val; guardar resultados e falhas.
5. **Comprovar execução:** capturar SSAO OFF/ON reais, testar alterações em F2, perf em F3, resize, shutdown e validation layers; documentar os dados do hardware.
6. **Corrigir e evoluir somente o necessário:** uma alteração por vez em branch de trabalho, com regressão. Não substituir automaticamente os 12 samples/blur 3×3 da engine pelos 32/5×5 do projeto acadêmico.
7. **Finalizar:** relacionar cada requisito do professor à evidência real, explicar diferenças do pseudocódigo, citar autoria original/licença, registrar os commits e preparar pacote de entrega.

## Prompt curto para Codex

> Leia `AGENTS.md`, `docs/SSAO_ORDEM_DE_EXECUCAO.md`, `docs/SSAO_ESTADO_ATUAL.md` e `docs/ENTREGA_SSAO.md` no fork da Eruption Engine. Continue na primeira ordem pendente sem recriar o motor nem alterar a main. Valide cada etapa com comandos e evidências reais, atualize o registro de progresso, e ao fim apresente SHA, alterações, testes executados, falhas e próxima ordem. Não invente FPS ou imagens, e diferencie a implementação da engine dos PDFs acadêmicos.

## Instalar as ordens dentro do clone

No Windows, use `INSTALAR_ORDENS_NO_FORK.bat`. No Linux, `bash INSTALAR_ORDENS_NO_FORK.sh`. Os scripts copiam arquivos para a raiz de `eruption-engine/` sem sobrescrever arquivos existentes e sem publicar alterações no GitHub.
