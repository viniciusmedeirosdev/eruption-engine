# 4. Windows / WSL2: edição não é execução nativa garantida

O computador Windows é perfeitamente adequado para **Git, VS Code, editar shaders e documentação**. O fork contém código e scripts destinados ao Linux. Não presuma que é um projeto Visual Studio/MSVC pronto para Windows apenas porque o Vulkan SDK existe para Windows.

## Etapa A — obter código no Windows

1. Instale Git for Windows e VS Code (opcional). Abra PowerShell nesta pasta.
2. Rode `powershell -ExecutionPolicy Bypass -File .\scripts\iniciar_windows.ps1`.
3. Abra `eruption-engine/` no VS Code; preserve `LICENSE` e `NOTICE` do repositório clonado.
4. Crie commits numa branch. O script não modifica `main` e só cria o clone se não houver um no destino.

## Etapa B — executar Linux

Opção mais previsível: Ubuntu 24.04 numa instalação real (nativa) com driver Vulkan do fabricante.

WSL2 com Ubuntu 24.04 pode servir, **mas é condicional**. Verifique `wsl --status` no Windows e, dentro do Ubuntu WSL, `vulkaninfo --summary`, versão de Vulkan, apresentação de janela e features exigidas. Muitas instalações do WSL expõem implementação/recursos diferentes do driver Linux nativo. A presença de uma RTX 2080 Ti no Windows não significa que o driver exposto via WSL fornecerá tudo que a engine exige.

Se WSL falhar na criação de surface ou device, prefira Linux nativo, máquina de laboratório ou ambiente Ubuntu com GPU e display Vulkan adequados. Não considere usar `xvfb` sozinho como solução para funcionalidades Vulkan não expostas.

## Arquivos e caminhos

No terminal PowerShell, `scripts/iniciar_windows.ps1`; no Ubuntu/WSL, `bash scripts/iniciar_linux.sh`. Não abra um diretório do Windows como sistema de arquivos Linux sem considerar impactos de permissões e I/O no build. Em geral um clone dentro de `$HOME` do Ubuntu é melhor para compilar em WSL.

## Alternativa: GitHub Actions

Os workflows do fork (`.github/workflows/ssao-build.yml` e `ssao-runtime.yml`) representam verificações automatizadas em Ubuntu. Eles não validam o driver gráfico/Windows da máquina final e não substituem uma demonstração local. Artefatos do Actions têm retenção limitada.
