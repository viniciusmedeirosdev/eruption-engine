# Critérios objetivos de aceite — SSAO/Vulkan

| Código | Critério | Evidência mínima | Impedimento legítimo |
|---|---|---|---|
| A00 | Trabalho preservado | SHA, branch, git status/remote | checkout inacessível |
| A01 | Arquitetura inspecionada | tabela arquivo/função por requisito Vulkan | arquivo não encontrado |
| A02 | Ferramentas e device identificados | `vulkaninfo --summary`, SO, toolchain | driver Vulkan ausente |
| A03 | Build/Shader OK | logs e código de retorno de build, CTest e spirv-val | dependência/compile falhou |
| A04 | Runtime ON/OFF | prints autênticos e logs de ambos, mesma câmera/cena | dispositivo/apresentação indisponível |
| A05 | Vulkan validation | logs com VUID e resultado, incluindo resize/shutdown | indisponível no hardware/ambiente |
| A06 | Evolução sem regressão | comparação e testes de mudança específica | não exigida; fase opcional |
| A07 | Entrega pronta para revisão | enunciado conferido, relatório, fontes, autoria, evidências | enunciado/arquivos faltantes |

## Situações que NÃO são provas de aceite

- `spirv-val` sozinho **não** prova ausência de bugs C++ nem qualidade de imagem.
- Build verde de CI **não** prova performance ou display em outro sistema.
- Screenshot artificial ou imagem da internet **não** é evidência de execução própria.
- Ausência de VUID nos logs disponíveis **não** prova ausência de VUID em qualquer fluxo.
- Arquivos da Entrega 3 em GLSL são **pseudocódigo acadêmico**, não os shaders atualmente carregados pelo motor.

## Entrega do agente em cada fase

- Status (`concluída`/`pendente`/`bloqueada`) e número da ordem.
- Ambiente + commit + argumentos utilizados.
- Lista de arquivos editados e por quê; comandos e seus códigos de saída.
- Evidências com **caminhos reais**, resultados observados, falhas e severidade.
- Próxima ordem a executar, sem prometer execução assíncrona.
