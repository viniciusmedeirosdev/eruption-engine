# 9. Git/GitHub — processo de desenvolvimento seguro

Repositórios: `origin` = https://github.com/viniciusmedeirosdev/eruption-engine.git; `upstream` = https://github.com/eruptionlabs/eruption-engine.git.

## Começar

```bash
cd eruption-engine
git status --short
git remote -v
git rev-parse HEAD
git switch -c trabalho-ssao-2026
```

## Sincronizar sem perder trabalho

```bash
git fetch origin
git fetch upstream
git log --oneline --decorate --graph -12
git log --oneline --left-right origin/main...upstream/main
```

**Não** faça `git pull upstream main` às cegas no branch de entrega. O upstream pode ter mudado depois da criação do fork e pode entrar em conflito com as alterações SSAO do grupo. Primeiro faça uma branch de integração, teste lá e abra PR antes de mesclar.

## Modificar arquivos

```bash
git diff -- src/renderer/DeferredLighting.cpp shaders/lighting/ssao.frag
git add src/renderer/DeferredLighting.cpp shaders/lighting/ssao.frag
git commit -m "test: melhora controle de SSAO e evidencia"
git push -u origin trabalho-ssao-2026
```

Evite adicionar `build*/`, `evidencias/` pesadas, binários, `.spv` gerados, mapas GLB e credenciais como código-fonte; revise `.gitignore`. Evidências devem ir no pacote de entrega ou artifacts do Actions quando adequado, com autor/data/commit.

## Verificações em PR

- `SSAO build and validation` compilou engine, CTest e `spirv-val`?
- `SSAO software rendering check` rodou e publicou evidências?
- Os artefatos existem e podem ser baixados enquanto a retenção permitir?
- Câmera/driver real de apresentação foi testado?
- A documentação foi atualizada para mudanças de shader/UBO/dynamic rendering?

O GitHub Actions não é substituto de aprovação da banca nem comprova execução física na máquina do usuário.
