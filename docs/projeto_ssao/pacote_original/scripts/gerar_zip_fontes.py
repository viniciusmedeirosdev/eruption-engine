#!/usr/bin/env python3
"""Gera ZIP de código REAL do checkout e adiciona a documentação do pacote.
Não contém .git, binários do build, GLB baixado nem dependências de submódulos.
Para histórico Git ou submódulos completos, use git clone --recurse-submodules.
"""
import subprocess,sys,zipfile,hashlib,datetime
from pathlib import Path
base=Path(__file__).resolve().parent.parent
repo=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else base/'eruption-engine'
if not (repo/'.git').exists():
    raise SystemExit('Repositório não clonado. Rode o script iniciar_* primeiro.')
def git(*args):
    r=subprocess.run(['git','-C',str(repo),*args],capture_output=True,check=True)
    return r.stdout
sha=git('rev-parse','HEAD').decode().strip()
raw=git('ls-files','-z').split(b'\0')
paths=[Path(x.decode('utf-8','surrogateescape')) for x in raw if x]
output=base/f'SSAO_FONTE_E_DOCUMENTOS_{sha[:10]}.zip'
count=0;missing=[]
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for rel in paths:
        f=repo/rel
        if f.is_file():
            z.write(f,arcname=str(Path('eruption-engine')/rel));count+=1
        else:
            missing.append(str(rel))
    for f in (base/'documentacao').rglob('*'):
        if f.is_file(): z.write(f,arcname=str(f.relative_to(base)))
    for f in (base/'referencias').rglob('*'):
        if f.is_file(): z.write(f,arcname=str(f.relative_to(base)))
    for folder in ('ordens_para_fork',):
        for f in (base/folder).rglob('*'):
            if f.is_file(): z.write(f,arcname=str(f.relative_to(base)))
    for name in ('LEIA_PRIMEIRO.md','CONTINUAR_AQUI.md','ORDENS_DE_CONTINUIDADE.md','AGENTS.md','INSTALAR_ORDENS_NO_FORK.sh','INSTALAR_ORDENS_NO_FORK.bat'):
        z.write(base/name,arcname=name)
    for name in ('scripts/instalar_ordens_no_fork.py',):
        z.write(base/name,arcname=name)
    notes=(f'Commit: {sha}\nOrigem: https://github.com/viniciusmedeirosdev/eruption-engine\n'
           'Este ZIP NÃO inclui .git nem submodulos aninhados. Para continuar com controle de versao: git clone --recurse-submodules.\n'
           'Nao inclui assets baixados, dependencias externas ou executaveis gerados.\n'
           f'Arquivos Git encontrados: {count}\nArquivos de checkout ausentes: {missing}\n')
    z.writestr('INFORMACOES_DO_CHECKOUT.txt',notes)
print(f'Criado: {output}\nArquivos rastreados incluidos: {count}; faltantes: {len(missing)}')
if missing:
    print('Atenção: alguns arquivos rastreados não estavam presentes:',*missing,sep='\n- ')
