# Orientações para agentes — pacote SSAO/Vulkan

Escopo: esta pasta `SSAO_Vulkan_Eruption_Projeto/` reúne documentação acadêmica e um checkout `eruption-engine/` que pode ser clonado dentro dela.

**Antes de modificar qualquer código:** leia `CONTINUAR_AQUI.md`, `LEIA_PRIMEIRO.md`, `documentacao/08_PLANO_DE_CONTINUIDADE.md`, `documentacao/07_DIFERENCAS_COM_ENTREGAS_1_A_3.md`. Se existir `eruption-engine/`, leia também `eruption-engine/AGENTS.md` e `eruption-engine/docs/ENTREGA_SSAO.md`.

**Regras inegociáveis**: continuar o fork, não recriar a engine; não presumir que testou GPU sem execução; não fabricar evidências/FPS; manter LICENSE/NOTICE e reconhecer a origem do código; não dar push/merge em `main` sem revisão; preservar quaisquer mudanças locais preexistentes; nunca apagar arquivos, caches, evidências ou builds sem autorização explícita; validar compilação, shaders, runtime, resize e liberação de recursos.

As instruções específicas para inserir no **fork** (onde a edição de C++/GLSL acontecerá) estão em `ordens_para_fork/`. Para instalá-las, executar `python3 scripts/instalar_ordens_no_fork.py` ou os lançadores da raiz. Nunca escreva nos PDFs históricos como se fossem o código realmente executado.
