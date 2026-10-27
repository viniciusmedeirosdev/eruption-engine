# Materiais acadêmicos e continuidade do SSAO/Vulkan

Este diretório reúne o material acadêmico e as instruções de continuação do projeto a partir do fork existente:
https://github.com/viniciusmedeirosdev/eruption-engine

## Comece por aqui
- `../../AGENTS.md`: instruções operacionais do agente de código.
- `../SSAO_ORDEM_DE_EXECUCAO.md`: sequência obrigatória das tarefas.
- `../SSAO_ESTADO_ATUAL.md`: histórico, status e itens não validados.
- `../SSAO_CRITERIOS_DE_ACEITE.md`: critérios verificáveis para finalizar.
- `../SSAO_PROMPTS_PARA_AGENTES.md`: prompts para sessões de desenvolvimento.
- `../ENTREGA_SSAO.md`: relatório da implementação existente e CI histórico.
- `pacote_original/LEIA_PRIMEIRO.md`: guia completo do pacote gerado.

## Materiais das entregas acadêmicas
- `pacote_original/documentacao/entregas_academicas/Entrega_1_e_2_TEXTO_EXTRAIDO.txt` 
- `pacote_original/documentacao/entregas_academicas/Entrega_3_TEXTO_EXTRAIDO.txt`
- PDFs históricos são preservados no pacote local; a publicação destes arquivos binários é uma etapa distinta.
- O fork já contém C++/GLSL, assets rastreados, scripts Vulkan e workflows de build.

## Atenção às diferenças técnicas
A proposta acadêmica anterior continha 32 amostras e filtro bilateral 5×5.
O fork executável documentado emprega kernel de 12 amostras e blur 3×3 em meia resolução.
Não declarar equivalência entre o pseudocódigo e o motor sem explicitar a adaptação.

## Como executar (Ubuntu 24.04 x86_64)
```bash
git clone --recurse-submodules https://github.com/viniciusmedeirosdev/eruption-engine.git
cd eruption-engine
bash tools/build_ssao.sh
bash tools/ssao_demo.sh capture
```
Requisitos, limitações e métodos de validação: `../ENTREGA_SSAO.md`.
Somente marque fases como validadas após execução real; não invente resultados de GPU, imagens ou FPS.

## Proveniência
A Eruption Engine original é de eruptionlabs e é licenciada sob Apache-2.0. 
As correções do fork e os materiais acadêmicos devem ter sua autoria diferenciada.
Consulte LICENSE e NOTICE. Este material não substitui a conferência do enunciado oficial.
