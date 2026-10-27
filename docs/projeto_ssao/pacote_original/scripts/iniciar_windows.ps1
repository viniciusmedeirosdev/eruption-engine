# A partir da raiz do pacote: powershell -ExecutionPolicy Bypass -File .\scripts\iniciar_windows.ps1
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Target = Join-Path $Root 'eruption-engine'
$Fork = 'https://github.com/viniciusmedeirosdev/eruption-engine.git'
$Upstream = 'https://github.com/eruptionlabs/eruption-engine.git'
$FreshClone = $false
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { throw 'Git não encontrado. Instale Git for Windows.' }
if ((Test-Path $Target) -and -not (Test-Path (Join-Path $Target '.git'))) {
    throw "Pasta $Target já existe mas não é checkout Git. Não será sobrescrita."
}
if (-not (Test-Path (Join-Path $Target '.git'))) {
    Write-Host 'Clonando o seu fork e os submódulos...'
    & git clone --recurse-submodules $Fork $Target
    if ($LASTEXITCODE -ne 0) { throw 'git clone falhou: confira conexão/permissões.' }
    $FreshClone = $true
} else {
    Write-Host 'Checkout existente detectado. Sem pull, sem checkout e sem alterações automáticas.'
}
Push-Location $Target
try {
    & git remote get-url upstream *> $null
    if ($LASTEXITCODE -ne 0) { & git remote add upstream $Upstream; if ($LASTEXITCODE -ne 0) { throw 'Falha ao adicionar upstream' } }
    Write-Host 'Commit atual:'
    & git rev-parse HEAD
    Write-Host 'Remotos:'
    & git remote -v
    Write-Host 'Estado:'
    & git status --short
    Write-Host 'Submódulos:'
    & git submodule status
} finally { Pop-Location }
if ($FreshClone) {
    Write-Host 'Instalando ordens de continuidade no checkout novo (sem sobrescrita)...'
    if (Get-Command py -ErrorAction SilentlyContinue) {
        & py -3 (Join-Path $PSScriptRoot 'instalar_ordens_no_fork.py') $Target
    } elseif (Get-Command python -ErrorAction SilentlyContinue) {
        & python (Join-Path $PSScriptRoot 'instalar_ordens_no_fork.py') $Target
    } else {
        Write-Host 'Aviso: Python indisponivel; execute INSTALAR_ORDENS_NO_FORK.bat depois.'
    }
} else {
    Write-Host 'Checkout existente preservado. Para instalar ordens manualmente use INSTALAR_ORDENS_NO_FORK.bat.'
}
Write-Host 'Clone pronto para edição. Para compilar, use Ubuntu 24.04 e consulte documentacao/03_INSTALAR_EXECUTAR_UBUNTU.md'
