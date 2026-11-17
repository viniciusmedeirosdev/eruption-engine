# Vulcao SSAO - instala o que faltar, compila e executa (Windows 10/11).
# Uso: execute "executar.bat" (duplo clique) ou: powershell -ExecutionPolicy Bypass -File executar.ps1
# Parametros opcionais: -Debug (liga a camada de validacao do Vulkan), -SoCompilar (nao abre o programa).
param([switch]$Debug, [switch]$SoCompilar)
$ErrorActionPreference = 'Stop'
$raiz  = $PSScriptRoot
$build = Join-Path $raiz 'build'
$tipo  = if ($Debug) { 'Debug' } else { 'Release' }

function Passo($texto) { Write-Host "`n==> $texto" -ForegroundColor Cyan }
function Falha($texto) { Write-Host "`nERRO: $texto" -ForegroundColor Red; exit 1 }

# Recarrega PATH e VULKAN_SDK do registro (programas recem-instalados alteram essas variaveis).
function AtualizarAmbiente {
    # Acrescenta (sem apagar) o que foi carregado antes, como o ambiente do MSVC.
    $env:Path = $env:Path + ';' + [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User')
    $sdk = [Environment]::GetEnvironmentVariable('VULKAN_SDK', 'Machine')
    if (-not $sdk) { $sdk = [Environment]::GetEnvironmentVariable('VULKAN_SDK', 'User') }
    if (-not $sdk) { $sdk = Get-ChildItem 'C:\VulkanSDK' -Directory -ErrorAction SilentlyContinue | Sort-Object Name | Select-Object -Last 1 -ExpandProperty FullName }
    if ($sdk) { $env:VULKAN_SDK = $sdk; $env:Path = "$sdk\Bin;$env:Path" }
}

function Instalar($id, $nome, [string[]]$extra = @()) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Falha "$nome nao esta instalado e o winget nao foi encontrado. Instale o 'Instalador de Aplicativo' pela Microsoft Store e tente de novo."
    }
    Write-Host "Instalando $nome (pode pedir permissao de administrador)..." -ForegroundColor Yellow
    winget install --id $id -e --silent --accept-package-agreements --accept-source-agreements @extra
    # -1978335189 = ja instalado / nenhuma atualizacao disponivel
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne -1978335189) { Falha "a instalacao de $nome falhou (codigo $LASTEXITCODE)." }
    AtualizarAmbiente
}

function AcharVisualStudio {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { return $null }
    & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}

AtualizarAmbiente

# --- 1. Git (o CMake usa para baixar GLFW, GLM, ImGui, cgltf e stb)
Passo 'Verificando Git'
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { Instalar 'Git.Git' 'Git' }

# --- 2. Compilador C++ (Visual Studio Build Tools com MSVC)
Passo 'Verificando compilador C++ (MSVC)'
$vs = AcharVisualStudio
if (-not $vs) {
    Instalar 'Microsoft.VisualStudio.2022.BuildTools' 'Visual Studio Build Tools 2022 (~3 GB, demora alguns minutos)' `
        @('--override', '--wait --passive --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended')
    $vs = AcharVisualStudio
    if (-not $vs) { Falha 'o compilador C++ nao foi encontrado apos a instalacao. Reinicie o computador e execute de novo.' }
}
# Carrega as variaveis de ambiente do MSVC (equivale a abrir o "x64 Native Tools Command Prompt").
$env:Path = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer;$env:Path" # vcvars usa o vswhere
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
cmd /c "`"$vcvars`" >nul && set" | ForEach-Object {
    if ($_ -match '^([^=]+)=(.*)$') { Set-Item -Path "env:$($Matches[1])" -Value $Matches[2] }
}
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) { Falha 'nao foi possivel carregar o ambiente do MSVC.' }
if ($env:VULKAN_SDK) { $env:Path = "$env:VULKAN_SDK\Bin;$env:Path" }

# --- 3. CMake e Ninja (normalmente ja vem com o Build Tools)
Passo 'Verificando CMake e Ninja'
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { Instalar 'Kitware.CMake' 'CMake' }
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) { Instalar 'Ninja-build.Ninja' 'Ninja' }
foreach ($p in 'cmake', 'ninja') { if (-not (Get-Command $p -ErrorAction SilentlyContinue)) { Falha "$p nao encontrado apos a instalacao. Feche e abra o terminal e execute de novo." } }

# --- 4. Vulkan SDK (cabecalhos, vulkan-1.lib e o compilador de shaders glslc)
Passo 'Verificando Vulkan SDK'
if (-not $env:VULKAN_SDK -or -not (Test-Path "$env:VULKAN_SDK\Bin\glslc.exe")) {
    Instalar 'KhronosGroup.VulkanSDK' 'Vulkan SDK'
    if (-not $env:VULKAN_SDK -or -not (Test-Path "$env:VULKAN_SDK\Bin\glslc.exe")) { Falha 'Vulkan SDK nao encontrado apos a instalacao.' }
}
Write-Host "Vulkan SDK: $env:VULKAN_SDK"

# --- 5. Configurar e compilar (a primeira vez baixa as bibliotecas e o mapa de ~170 MB)
Passo "Configurando o projeto ($tipo)"
cmake -S $raiz -B $build -G Ninja "-DCMAKE_BUILD_TYPE=$tipo" -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if ($LASTEXITCODE -ne 0) { Falha 'a configuracao do CMake falhou (veja as mensagens acima).' }

Passo 'Compilando'
cmake --build $build
if ($LASTEXITCODE -ne 0) { Falha 'a compilacao falhou (veja as mensagens acima).' }

if ($SoCompilar) { Write-Host "`nPronto: $build\vulcao.exe" -ForegroundColor Green; exit 0 }

# --- 6. Executar
Passo 'Abrindo o Vulcao SSAO'
& (Join-Path $build 'vulcao.exe')
if ($LASTEXITCODE -ne 0) { Falha "o programa terminou com erro (codigo $LASTEXITCODE)." }
