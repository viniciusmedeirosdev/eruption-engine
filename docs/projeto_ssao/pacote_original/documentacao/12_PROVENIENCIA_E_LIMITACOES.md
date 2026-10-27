# 12. Proveniência dos arquivos e limites da verificação

## Fontes verificadas em 27/10/2026

- Fork: https://github.com/viniciusmedeirosdev/eruption-engine (público, fork de Eruption Engine, `main`).
- Commit do fork inspecionado: `7c053e6b5220d98af1af1462a4748df75d26a0c8`.
- Origem: https://github.com/eruptionlabs/eruption-engine (Apache-2.0 para o código, NOTICE e assets licenciados separadamente).
- Documentação primária: `docs/ENTREGA_SSAO.md` disponível no clone do fork.
- Actions históricos de build e runtime citados em `06_TESTES_E_EVIDENCIAS.md`, sucesso no commit `dc990c755add2dbd990505d06e65729de589a16d`, registrados em 08/10/2026.
- Entregas acadêmicas originais recuperadas da Biblioteca de arquivos da conversa: `SSAO_Entregas_1_e_2.pdf` (30 páginas) e `Entrega_3_Pseudocodigo_SSAO.pdf` (22 páginas). Essas cópias foram incluídas sem edição.

## Limites

- **Não** foi possível realizar `git clone` neste ambiente por falha de resolução de `github.com`. Este pacote entrega documentação + script de clone; **não** contém árvore C++ completa nem .git. Requer Git/internet no computador do usuário para reconstruir essa árvore.
- A engine Vulkan **não foi recompilada, aberta ou capturada** nesta sessão. Nenhuma imagem ou medição de FPS nova foi gerada.
- O enunciado acadêmico exato da etapa final não estava anexado nesta sessão; a conferência item a item ainda depende desse texto.
- O projeto de origem e o fork podem mudar depois desta data. Os scripts salvam commit e arquivo de auditoria para rastreabilidade.
- Artefatos GitHub Actions têm prazo de retenção; verifique disponibilidade diretamente no GitHub.
- Não foi possível recuperar arquivos-fonte LaTeX, zip original completo das entregas ou capturas separadas além do que os PDFs incorporam; esse material não é inventado.

## Evidência de cópia dos PDFs

Use os checksums SHA-256 do arquivo `MANIFESTO_SHA256.txt` na raiz para conferir os PDFs e os documentos deste pacote, e o script `auditar_repositorio.py` após clone para gerar o inventário do fork e identificar os arquivos relevantes.
