# Ordens de continuidade — execução sequencial para SSAO/Vulkan

**Regra operacional:** cumprir na ordem. A fase seguinte depende das evidências da anterior. Agente autorizado a investigar e corrigir problemas dentro de branch de trabalho, mas não a declarar sucesso sem executar/verificar. Não mesclar automaticamente no `main`.

## ORDEM 00 — Recuperar estado e proteger alterações

1. Rodar `pwd`, `git status --short`, `git branch --show-current`, `git rev-parse HEAD`, `git remote -v`, `git submodule status`.
2. Conferir se `origin` é `viniciusmedeirosdev/eruption-engine`. `upstream` serve apenas de referência inicial. Não fazer `pull` do upstream por padrão.
3. Identificar mudanças não commitadas, arquivos ignorados relevantes e novas features desde o snapshot. Não sobrescrever arquivos do usuário.
4. Abrir branch de trabalho caso a atual seja `main`: `git switch -c trabalho-ssao-continuacao` (verificar antes se já existe).
5. Atualizar `docs/SSAO_ESTADO_ATUAL.md` com branch, SHA, ambiente e status inicial.

**Aceite:** há SHA, branch, status registrado e nenhum trabalho perdido.

## ORDEM 01 — Estabelecer mapa técnico verificável

1. Ler `docs/ENTREGA_SSAO.md`, `CMakeLists.txt`, `.github/workflows/ssao-build.yml`, `.github/workflows/ssao-runtime.yml`.
2. Ler e mapear `VulkanContext` (instância, dispositivo, queues, swapchain, sync), `GBuffer`, `DeferredLighting` (SSAO/blur/composição), `Engine`/`Render` (loop), GLSL e UI.
3. Montar tabela **requisito → arquivo/função → estado → prova**. Assinalar `não inspecionado`, `encontrado no código` ou `testado em execução` distintamente.
4. Fazer registro claro de diferenças acadêmicas: 32→12 amostras; blur 5×5→3×3; RP0–3→dynamic rendering; `R16F`→`R8`; view space→world reconstruction; normal/depth reverse-Z no motor.

**Aceite:** tabela com referências reais, sem confundir proposta acadêmica e fonte atual.

## ORDEM 02 — Instalar e verificar ambiente

**Plataforma primária:** Ubuntu 24.04 x86_64; Windows/WSL2 apenas se driver, Vulkan presentation e features forem suportados. Requisitos: compilador C++17, CMake, glslc, Vulkan 1.3, validation layers, bibliotecas X11, AVX2 e dependências da engine.

1. Executar `vulkaninfo --summary`; registrar a GPU/driver e recursos disponíveis. Não concluir a partir do modelo da placa apenas.
2. Conferir as instruções de `docs/ENTREGA_SSAO.md` (instalação de dependências).
3. Confirmar rede para FetchContent e mapa de demonstração; nunca pular a falha do mapa como se o teste gráfico tivesse passado.

**Aceite:** versão do SO, compilador, ferramentas, device e limitações documentadas.

## ORDEM 03 — Compilar e testar baseline

```bash
set -o pipefail
bash tools/build_ssao.sh 2>&1 | tee build-ssao.log
# set -o pipefail preserva o status real do build no pipeline.
ctest --test-dir build-ssao --output-on-failure
spirv-val --target-env vulkan1.3 shaders/lighting/ssao.frag.spv
spirv-val --target-env vulkan1.3 shaders/lighting/ssao_blur.frag.spv
spirv-val --target-env vulkan1.3 shaders/lighting/ambient.frag.spv
```

- Preferir execução com `set -o pipefail`; build usa `BUILD_JOBS=2` por padrão e FetchContent pode depender da rede.
- Verificar existência de `assets/external/parana_field/parana_field.glb` e do executável real.
- Se falhar, registrar erro, causa, ambiente e tentativa mínima de correção; não seguir para visualização antes de recompilar.

**Aceite:** C++ e shaders compilados, CTest e spirv-val reportados individualmente; GLB e binário presentes.

## ORDEM 04 — Execução real e evidências SSAO ON/OFF

```bash
bash tools/ssao_demo.sh off
bash tools/ssao_demo.sh on
bash tools/ssao_demo.sh capture
find evidencias -type f | sort
```

1. Registrar commit, Vulkan device/driver, preset, mapa, horário fixo, câmera, resolução, raio e força.
2. Capturar a **mesma cena** com SSAO OFF e ON, sem imagens artificiais. Comparar contato, bordas, haloes e ruído.
3. Validar UI (F2) para SSAO raio/intensidade e painel de FPS (F3); reportar números somente se medidos.
4. Se janela indisponível, usar CI/headless apenas como evidência do ambiente testado; deixar teste desktop como pendente.

**Aceite:** PNGs reais legíveis, logs `off`/`on`, metadados, ausência de crashes. Anotar onde estão salvos; não anexar GLB nem executáveis no commit por padrão.

## ORDEM 05 — Validação Vulkan, sincronização e ciclo de vida

1. Executar Debug/validation layers: `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation bash tools/ssao_demo.sh capture`.
2. Conferir VUIDs, warnings, layouts, barreiras G-buffer→SSAO raw→blur→lighting, descriptor sets, framebuffer/swapchain.
3. Testar alteração de tamanho da janela, minimização/restauração se suportado, recriação de recursos, shutdown e reinício.
4. Se detectar erro: isolar VUID, função, commit e passos; corrigir em alteração mínima; repetir build e teste de regressão.

**Aceite:** logs analisados e resultados declarados por hardware/driver específico, não generalizados.

## ORDEM 06 — Estudo de qualidade e evolução (apenas se pedida/necessária)

1. Primeiro confirmar requisitos do enunciado atual; a versão existente já contém SSAO funcional no caminho validado de CI histórico.
2. Caso seja exigido alinhar número de amostras ou filtro, projetar modo configurável 12/32 e 3×3/5×5 com comparação A/B, benchmarks e teste de shaders.
3. Não substituir indiscriminadamente `LightingUBO`, format do G-buffer ou depth reverse-Z pelo layout acadêmico. Atualizar C++/GLSL/diagrama/documentação juntos.
4. Cada alteração deve passar CTest, spirv-val, runtime ON/OFF, validação Vulkan e inspeção visual.

**Aceite:** mudança justificável, compilável, sem regressões identificadas e com medição reproduzível ou limite declarado.

## ORDEM 07 — Relatório, entrega e versionamento

1. Revisar **enunciado textual da última entrega** item a item. Onde faltar documento da disciplina, marcar `aguardando enunciado` e não alegar conformidade total.
2. Documentar os códigos de terceiros e licença Apache-2.0, sem assumir autoria do motor original. Diferenciar o que é upstream do que foi alterado no fork.
3. Anexar evidências e scripts de reprodução, diagramas correspondentes à engine, resumo do experimento e limitações.
4. `git diff --check`, revisar `git status`, fazer commit somente do necessário na branch e abrir PR para revisão.
5. Atualizar `docs/SSAO_ESTADO_ATUAL.md` com commit final, testes, links válidos e próxima etapa.

**Aceite:** links, comandos, rastreabilidade, provas reais e checklist de docente; resultado nunca declarado perfeito por suposição.

## Como retomar após uma interrupção

Ler `AGENTS.md` + `docs/SSAO_ESTADO_ATUAL.md`; executar ORDEM 00 para conferir divergências; voltar à **primeira ordem pendente**. Não repetir correções já testadas nem refazer commits sem verificar o estado atual.
