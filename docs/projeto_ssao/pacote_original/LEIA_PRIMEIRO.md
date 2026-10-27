# SSAO em Vulkan — pacote de continuidade do projeto

**Projeto:** Ambient Occlusion em Espaço de Tela (SSAO), disciplina de Computação Gráfica, UNISANTOS, 2026.

**Repositório de continuidade:** https://github.com/viniciusmedeirosdev/eruption-engine
**Projeto original:** https://github.com/eruptionlabs/eruption-engine
**Referência verificada do fork em 27/10/2026:** branch `main`, commit `7c053e6b5220d98af1af1462a4748df75d26a0c8`.

## Importante — o que este ZIP é e o que não é

Este arquivo contém **os PDFs acadêmicos recuperados, documentação técnica de continuidade, roteiros de implantação, verificação e captura, e scripts para baixar o código-fonte oficial do fork**. Não é uma cópia offline integral de todos os arquivos do motor. Neste ambiente não foi possível baixar o arquivo de código e os assets do GitHub em formato de clone/ZIP. A pasta `eruption-engine/` será criada automaticamente **na sua máquina, com acesso à internet**, pelo script inicializador.

Não são incluídos binário executável, mapa GLB de demonstração (~170 MB no release), SPIR-V compilado, PNGs dos runs do Actions nem provas simuladas. Os scripts **não alegam ter rodado testes locais**. Há validação histórica registrada no Actions em 08/10/2026; veja `documentacao/06_TESTES_E_EVIDENCIAS.md`.

## Ordens de continuidade (NOVO)

Leia **`ORDENS_DE_CONTINUIDADE.md`** e **`CONTINUAR_AQUI.md`** para saber exatamente como passar o projeto ao Codex/Cline e continuar pela primeira etapa pendente. Este pacote também tem um `AGENTS.md` de escopo geral e a pasta `ordens_para_fork/`, com as ordens que podem ser instaladas **na raiz do checkout do GitHub**, incluindo `AGENTS.md`, prompts e status de execução.

Depois de clonar o fork: execute `INSTALAR_ORDENS_NO_FORK.bat` (Windows) ou `bash INSTALAR_ORDENS_NO_FORK.sh` (Linux). O instalador **não sobrescreve arquivos existentes nem faz push**. Um clone novo pelos inicializadores do pacote também recebe essas ordens automaticamente quando o instalador está disponível. Para retomar: abra `eruption-engine/` no VS Code/Codex e peça para seguir o `AGENTS.md` e `docs/SSAO_ORDEM_DE_EXECUCAO.md`.

## Comece por aqui

**Windows 10/11:** abra PowerShell nesta pasta e execute `powershell -ExecutionPolicy Bypass -File .\scripts\iniciar_windows.ps1`. Ele instala o repositório ao lado destes documentos, se Git estiver instalado; para **executar a engine**, use Ubuntu 24.04 com Vulkan gráfico funcional (máquina Linux ou WSL2, este último sujeito à compatibilidade).

**Ubuntu 24.04:** `bash scripts/iniciar_linux.sh`, depois leia `documentacao/03_INSTALAR_EXECUTAR_UBUNTU.md`. Os comandos de compilação são executados **dentro de `eruption-engine/`**, não na raiz deste pacote.

**Já tem repositório clonado?** Não sobrescreva seus arquivos; compare `git status`, adicione este ZIP à documentação local e siga `documentacao/09_FLUXO_GITHUB.md`.

## Estrutura

```text
SSAO_Vulkan_Eruption_Projeto/
├── LEIA_PRIMEIRO.md
├── ORDENS_DE_CONTINUIDADE.md (resumo + prompt pronto)
├── CONTINUAR_AQUI.md (ordens para pessoas e agentes)
├── AGENTS.md (orientações para Codex aberto nesta pasta)
├── INSTALAR_ORDENS_NO_FORK.bat / .sh
├── ordens_para_fork/ (AGENTS.md + docs para instalar no checkout)
├── documentacao/
│   ├── 01_CONTEXTO_E_HISTORICO.md
│   ├── 02_ARQUITETURA_E_FLUXO.md
│   ├── 03_INSTALAR_EXECUTAR_UBUNTU.md
│   ├── 04_WINDOWS_E_WSL2.md
│   ├── 05_MAPA_DOS_ARQUIVOS.md
│   ├── 06_TESTES_E_EVIDENCIAS.md
│   ├── 07_DIFERENCAS_COM_ENTREGAS_1_A_3.md
│   ├── 08_PLANO_DE_CONTINUIDADE.md
│   ├── 09_FLUXO_GITHUB.md
│   ├── 10_CHECKLIST_ENTREGA_FINAL.md
│   ├── 11_PROMPT_PARA_AGENTE_CODIGO.md
│   ├── 12_PROVENIENCIA_E_LIMITACOES.md
│   └── entregas_academicas/
│       ├── SSAO_Entregas_1_e_2.pdf
│       └── Entrega_3_Pseudocodigo_SSAO.pdf
├── scripts/
│   ├── iniciar_windows.ps1
│   ├── iniciar_linux.sh
│   ├── verificar_ambiente_ubuntu.sh
│   ├── testar_projeto_linux.sh
│   ├── capturar_evidencias_linux.sh
│   ├── auditar_repositorio.py
│   ├── gerar_zip_fontes.py
│   └── instalar_ordens_no_fork.py
├── referencias/
│   ├── pipeline_ssao.dot
│   └── README_REFERENCIAS.md
└── eruption-engine/  (criado após o clone; código do fork)
```

**Convenção:** scripts são complementares e não substituem o `tools/build_ssao.sh` ou o `tools/ssao_demo.sh` do fork. Não modifique o `main` automaticamente; trabalhe em uma branch.
