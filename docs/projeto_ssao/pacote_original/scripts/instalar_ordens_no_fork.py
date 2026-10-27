#!/usr/bin/env python3
"""Instala documentação de continuidade no clone local sem sobrescrever arquivos.

Uso: python3 scripts/instalar_ordens_no_fork.py [diretorio-do-fork]
Não faz clone, não altera código fonte e não executa comandos Git de escrita.
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path


def run():
    ap = argparse.ArgumentParser(description=__doc__)
    default_repo = Path(__file__).resolve().parent.parent / 'eruption-engine'
    ap.add_argument('repo', nargs='?', type=Path, default=default_repo, help='caminho do checkout Git')
    args = ap.parse_args()
    base = Path(__file__).resolve().parent.parent
    source = base / 'ordens_para_fork'
    dest = args.repo.resolve()
    if not source.is_dir():
        ap.error(f'Pasta de origem não existe: {source}')
    if not (dest / '.git').exists():
        ap.error(f'Repositório ausente em {dest}; clone o fork primeiro.')
    remote = subprocess.run(['git','-C',str(dest),'remote','get-url','origin'], text=True, capture_output=True)
    if remote.returncode != 0:
        ap.error('Checkout sem remote origin; verifique antes de instalar as ordens.')
    origin = remote.stdout.strip().lower()
    if not any(z in origin for z in ('github.com/viniciusmedeirosdev/eruption-engine', 'github.com:viniciusmedeirosdev/eruption-engine')):
        ap.error(f'Origin não parece ser o fork esperado: {remote.stdout.strip()}')
    added, identical, skipped = [], [], []
    for p in sorted(source.rglob('*')):
        if not p.is_file():
            continue
        rel = p.relative_to(source)
        target = dest / rel
        if target.exists():
            if target.is_file() and p.read_bytes() == target.read_bytes():
                identical.append(str(rel))
            else:
                skipped.append(str(rel))
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(p, target)
        added.append(str(rel))
    print(f'Repositório: {dest}')
    print(f'Origin: {remote.stdout.strip()}')
    print(f'Arquivos criados ({len(added)}):', *(f'  + {s}' for s in added), sep='\n')
    print(f'Já idênticos ({len(identical)}):', *(f'  = {s}' for s in identical), sep='\n')
    if skipped:
        print('Atenção: arquivos preexistentes diferentes, PRESERVADOS:', *(f'  ! {s}' for s in skipped), sep='\n')
    print('\nPróximo passo: abra o checkout no editor e leia AGENTS.md. Confira `git status --short` antes de commitar.')
    return 2 if skipped else 0

if __name__ == '__main__':
    sys.exit(run())
