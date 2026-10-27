# CONTINUAR AQUI — ordens de continuidade do SSAO/Vulkan

Este roteiro foi incluído no pacote do projeto em 27/10/2026. É o ponto de entrada para **um desenvolvedor ou agente (Codex/Cline)** continuar o fork existente, sem reconstruir a aplicação do zero.

**Repositório:** https://github.com/viniciusmedeirosdev/eruption-engine  
**Base histórica observada:** `main` em `7c053e6b5220d98af1af1462a4748df75d26a0c8` (o HEAD atual deve ser confirmado pelo Git; este valor não é automaticamente atual).  
**Fonte original:** https://github.com/eruptionlabs/eruption-engine (Apache-2.0; manter LICENSE e NOTICE). 

## Ordem rápida para humanos

1. Leia `LEIA_PRIMEIRO.md` e `documentacao/08_PLANO_DE_CONTINUIDADE.md`.
2. Clone o fork com `INICIAR_NO_WINDOWS.bat` ou `INICIAR_NO_LINUX.sh` (não sobrescrevem checkout existente).
3. Instale as ordens **dentro do checkout** executando `INSTALAR_ORDENS_NO_FORK.bat` (Windows) ou `bash INSTALAR_ORDENS_NO_FORK.sh` (Linux). Em checkout novo os iniciadores também podem instalar automaticamente sem substituir arquivos existentes.
4. Abra `eruption-engine/` no VS Code/Codex. O arquivo `eruption-engine/AGENTS.md` fará o agente ler a sequência de tarefas em `docs/SSAO_ORDEM_DE_EXECUCAO.md`.
5. Use `docs/SSAO_PROMPTS_PARA_AGENTES.md` para instruções de cada etapa, uma por vez.
6. A cada etapa, atualize `docs/SSAO_ESTADO_ATUAL.md` com evidência verdadeira, próximos passos e impedimentos.
7. **Só depois de validar**: commit em branch, push e PR. Nada de alterar `main` nem fazer merge cego de `upstream`.

## Copiar manualmente para repositório já clonado

**PowerShell**, na raiz deste pacote:

```powershell
py -3 .\scripts\instalar_ordens_no_fork.py
# Ou: python .\scripts\instalar_ordens_no_fork.py
```

**Linux**, na raiz deste pacote:

```bash
python3 scripts/instalar_ordens_no_fork.py
```

O instalador copia `ordens_para_fork/AGENTS.md` e `ordens_para_fork/docs/*.md` para `eruption-engine/` sem substituir qualquer arquivo preexistente. Os arquivos ficam locais/não rastreados até você adicioná-los com Git. **Não faz git push nem modifica os shaders.**

## Status real: o que NÃO declarar sem repetir os testes

- Em 08/10/2026, os dois workflows de CI (`SSAO build and validation` e `SSAO software rendering check`) terminaram com `success` no commit `dc990c755add2dbd990505d06e65729de589a16d`.
- Isso **não comprova** o funcionamento da sua GPU, fidelidade do SSAO em todos os cenários, ausência universal de bugs, FPS nem funcionamento de todos os presets.
- Em 27/10/2026 o fork estava no commit de documentação `7c053e6...` e incluía SSAO real da engine com **12 amostras**, **blur 3×3**, **R8/meia resolução**. Os PDFs acadêmicos propunham **32 amostras**, **blur 5×5**, e arquitetura distinta. Leia `documentacao/07_DIFERENCAS_COM_ENTREGAS_1_A_3.md` antes de alterar.
- Não há comprovação de que os fontes `.tex`, imagens originais dos jogos ou enunciado da entrega final estejam no pacote. Se o professor exigir, obtê-los e identificá-los sem inventar evidências.

## Arquivos das ordens (modelo para instalar no fork)

```text
ordens_para_fork/
├── AGENTS.md                          # instrução lida pelo agente na raiz do repo
└── docs/
    ├── SSAO_ORDEM_DE_EXECUCAO.md       # etapas numeradas e bloqueios
    ├── SSAO_CRITERIOS_DE_ACEITE.md     # o que provar antes de concluir
    ├── SSAO_ESTADO_ATUAL.md            # registro contínuo e pendências
    └── SSAO_PROMPTS_PARA_AGENTES.md    # prompts por fase + handoff
```

Os arquivos existentes no repositório original NÃO são duplicados no ZIP. `docs/ENTREGA_SSAO.md` já está no fork após o clone e é o documento técnico de referência da implementação.
