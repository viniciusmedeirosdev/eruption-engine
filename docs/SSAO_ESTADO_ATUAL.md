# Registro vivo de continuidade — SSAO/Vulkan

> Atualizar este arquivo após **cada sessão** de desenvolvimento/validação, sem preencher resultados não obtidos. Não apagar o histórico de problemas relevantes.

## Referência histórica (antes da nova execução)

- Snapshot consultado em 27/10/2026: branch `main`, SHA `7c053e6b5220d98af1af1462a4748df75d26a0c8`.
- GitHub Actions em 08/10/2026: build `37744003559` = success; runtime `37744189560` = success; commit executado `dc990c755add2dbd990505d06e65729de589a16d`.
- CI runtime utilizou Ubuntu 24.04, software Vulkan llvmpipe, SSAO OFF/ON e validation layers. Não inferir teste GPU local.
- Implementação observada: GLSL SSAO 12 amostras, blur bilateral 3×3 e renderização em meia resolução; PDFs acadêmicos com 32 amostras / 5×5 são projeto anterior, não implementação idêntica.

## Sessão nova — preencher após executar

| Campo | Valor |
|---|---|
| Data/hora e responsável | PENDENTE |
| Branch | PENDENTE |
| SHA do HEAD | PENDENTE |
| `git status --short` inicial | PENDENTE |
| SO/compilador/CMake | PENDENTE |
| Device/driver Vulkan (`vulkaninfo`) | PENDENTE |
| Ordem atual (00–07) | 00 — PENDENTE |
| Build, exit code, log | NÃO EXECUTADO NESTA SESSÃO |
| CTest, resultados | NÃO EXECUTADO NESTA SESSÃO |
| spirv-val | NÃO EXECUTADO NESTA SESSÃO |
| SSAO OFF/ON, arquivos | NÃO EXECUTADO NESTA SESSÃO |
| Validação/VUIDs/resize | NÃO EXECUTADO NESTA SESSÃO |
| Arquivos alterados | NENHUMA ALTERAÇÃO REGISTRADA |
| Bloqueios e decisões | PENDENTE |
| Próxima ordem exata | ORDEM 00 |

## Lista de tarefas

- [ ] ORDEM 00 — confirmar checkout, criar branch segura e registrar estado.
- [ ] ORDEM 01 — auditoria do pipeline/requisitos em arquivos e funções.
- [ ] ORDEM 02 — checar ambiente/Vulkan.
- [ ] ORDEM 03 — build + CTest + spirv-val.
- [ ] ORDEM 04 — runtime SSAO ON/OFF e capturas reais.
- [ ] ORDEM 05 — validação Vulkan, resize e encerramento.
- [ ] ORDEM 06 — melhorias se necessárias (opcional).
- [ ] ORDEM 07 — documentação final, proveniência e revisão GitHub.

## Handoff (preencher antes de encerrar)

**O que foi concluído nesta sessão:** PENDENTE.  
**Que testes foram realmente executados:** PENDENTE.  
**Falhas reproduzíveis / links de logs:** PENDENTE.  
**Próxima ação imediata:** executar ORDEM 00 e depois a primeira etapa pendente.  
**Restrições do ambiente:** PENDENTE.

## Sessao 27/10/2026 (Claude Code) — PAUSADA a pedido do usuario

- Branch local `feat/ssao-entrega-final` (criada em `ed557ba`). **Nada commitado nem publicado ainda.**
- Ambiente: Windows 10, RTX 2080 Ti (driver 616.92, Vulkan 1.4), MSYS2 UCRT64 em `D:\msys64` (GCC 16.2, CMake 4.4), build em `build-win/`.
- Feito e testado na GPU: porte Windows (Platform.hpp etc.), build + CTest 2/2 + spirv-val OK; SSAO com texelFetch;
  kernel academico opcional (32 amostras/5x5); visualizacao do buffer SSAO (ERUPTION_TEST_PBR_DEBUG=128);
  A/B na mesma execucao (ERUPTION_TEST_SSAO_AB); eventos de roteiro (ssao*, size, minimize, restore);
  correcoes de swapchain/resize/minimizar e de sincronizacao. Roteiro completo com validation + sync validation: 0 erros
  (resta 1 warning benigno: saida de velocidade location 5 sem attachment).
- GPU (vista padrao, 1920x1061): iluminacao diferida OFF 0,93 ms; SSAO 12/3x3 1,10 ms; 32/5x5 1,28 ms.
- Demonstracao para aula: `D:\Processamento de Imagens\DEMO_SSAO_AULA\` (fora do repositorio).
- PENDENTE: revisar diff, commits, push, PR, documentacao final, CI Ubuntu; apagar `D:\Processamento.log` e `eruption-engine\Imagens\` (sobras de um comando meu).
