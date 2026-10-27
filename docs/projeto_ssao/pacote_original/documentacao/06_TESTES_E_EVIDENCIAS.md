# 6. Testes, logs e evidências autênticas

## Histórico verificado no GitHub (08/10/2026)

- **Build e validação:** https://github.com/viniciusmedeirosdev/eruption-engine/actions/runs/37744003559 — estado concluído, `success`, código testado no commit `dc990c755add2dbd990505d06e65729de589a16d`.
- **Execução gráfica via software:** https://github.com/viniciusmedeirosdev/eruption-engine/actions/runs/37744189560 — estado concluído, `success`, mesmo commit.
- Workflow build realiza compilação, CTest e `spirv-val --target-env vulkan1.3` sobre SSAO, blur e ambiente; publica um tar Linux.
- Workflow runtime usa Ubuntu 24.04, driver Mesa de software (llvmpipe), `xvfb`, preset low, `ERUPTION_TEST_NO_EFFECTS=1`, `ERUPTION_TEST_NO_SHADOWS=1`, `ERUPTION_RENDER_SCALE=0.5`, screenshot no terceiro frame e layers de validação. Alterna SSAO OFF/ON, verifica capturas e falha se encontrar mensagens `Validation Error` ou `VUID-`.

**Não infira** que isso provou todas as GPUs, FPS em tempo real ou funcionamento na sua máquina. Além disso, o commit inspecionado no `main` em 27/10/2026 é `7c053e6...` (documentação posterior), então descreva explicitamente qual commit foi testado.

## Teste local reproduzível

```bash
cd eruption-engine
bash tools/build_ssao.sh
bash tools/ssao_demo.sh capture
find evidencias -type f | sort
```

Expectativa de uma rodada de captura: `ssao-off.png`, `ssao-on.png`, `ssao-off.log`, `ssao-on.log`, `parametros.txt`. Para logs de validação, use `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`. Os scripts reais podem mudar; confira as mensagens emitidas.

## Metadados mínimos junto às evidências

Crie `README_EVIDENCIAS.md` ao lado das capturas contendo: data/hora, sistema, driver e dispositivo Vulkan, commit SHA, resolução, preset, raio, intensidade, ângulo da câmera e mapa, resultado da validação, FPS ou tempos somente se medidos com metodologia consistente, nome dos responsáveis pelas capturas.

## Lista de QA visual

| Ação | Resultado esperado |
|---|---|
| SSAO OFF | contatos menos escurecidos, sem ruído do AO |
| SSAO ON | oclusão próxima a paredes, cantos e objetos, sem halos exagerados |
| Mudar raio/força | mudança de escala e intensidade, sem travar |
| Redimensionar | render continua, sem distorções nem validation errors |
| Fechar e reabrir | shaders/descriptors recriados quando necessário |
| Repetir no mesmo frame/câmera | comportamento qualitativamente reprodutível |
| Logs com validação | sem `Validation Error`/`VUID-` do Vulkan |

**Importante:** não gere imagens artificiais como se fossem capturas do programa. Se a engine não rodar, entregue logs reais e explique as limitações; os artefatos CI podem apoiar mas não substituir transparência.
