#!/usr/bin/env python3
"""Auditoria estática de arquivos e estado Git; NÃO é teste Vulkan.
Uso: python3 scripts/auditar_repositorio.py [caminho/para/eruption-engine]
"""
import json, subprocess, sys
from pathlib import Path
from datetime import datetime, timezone

base=Path(__file__).resolve().parent.parent
repo=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else base/'eruption-engine'
required=[
    'docs/ENTREGA_SSAO.md','CMakeLists.txt','LICENSE','NOTICE',
    'src/main.cpp','src/renderer/VulkanContext.cpp','src/renderer/VulkanContext.hpp',
    'src/renderer/GBuffer.cpp','src/renderer/DeferredLighting.cpp',
    'src/renderer/DeferredLighting.hpp','src/core/Engine.cpp','src/core/Render.cpp',
    'shaders/lighting/ssao.frag','shaders/lighting/ssao_blur.frag',
    'shaders/lighting/ambient.frag','shaders/lighting/directional.frag',
    'tools/build_ssao.sh','tools/ssao_demo.sh','tools/fetch_demo_map.sh',
    '.github/workflows/ssao-build.yml','.github/workflows/ssao-runtime.yml']

def git(*args):
    try:
        p=subprocess.run(['git','-C',str(repo),*args],text=True,capture_output=True,check=False)
        return p.stdout.strip() if p.returncode==0 else None
    except FileNotFoundError:
        return None

exists=repo.is_dir() and (repo/'.git').exists()
present={p:(repo/p).is_file() for p in required}
report={
    'tipo':'AUDITORIA ESTATICA; NAO executa codigo nem garante funcionamento Vulkan',
    'criado_utc':datetime.now(timezone.utc).isoformat(),
    'repo':str(repo),'git_checkout':exists,
    'commit':git('rev-parse','HEAD') if exists else None,
    'branch':git('branch','--show-current') if exists else None,
    'status_porcelain':git('status','--short') if exists else None,
    'origin':git('remote','get-url','origin') if exists else None,
    'upstream':git('remote','get-url','upstream') if exists else None,
    'arquivos_criticos':present,
    'faltantes':[p for p,ok in present.items() if not ok],
    'arquivos_rastreados_total':len((git('ls-files') or '').splitlines()) if exists else None
}
output_dir=base/'relatorios'; output_dir.mkdir(exist_ok=True)
report_path=output_dir/'auditoria_repositorio.json'
report_path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
print('Relatório salvo em',report_path)
sys.exit(0 if exists and all(present.values()) else 1)
