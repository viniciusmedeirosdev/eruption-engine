# 10. Checklist final para o grupo

## Documentação e autoria

- [ ] Entregas 1–3 anexadas, legíveis, consistentes com os nomes do grupo.
- [ ] Enunciado exato da etapa final transcrito/conferido item a item.
- [ ] Explicação de por que foi usado um **fork de engine existente**, com referência de código original.
- [ ] Link GitHub, branch e SHA específicos indicados no relatório.
- [ ] Licença Apache-2.0 e avisos NOTICE preservados; licenças de assets checadas.
- [ ] Diferenças 32 vs 12 amostras, 5x5 vs 3x3, dynamic rendering vs render passes tradicionais declaradas.

## Código / Vulkan

- [ ] Instância, extensions/layers, physical/logical device, queues e surface.
- [ ] Swapchain, image views e imagens internas.
- [ ] Dynamic rendering/pipelines/descriptor sets/UBOs e shaders integrados.
- [ ] Vertex/index buffers e comandos para geometria, fullscreen e iluminação.
- [ ] Command pools/buffers e sincronização sem erros de validação.
- [ ] Loop de renderização e cleanup comprovados.
- [ ] Build C++17, GLSL->SPIR-V e testes CTest passando.

## Execução demonstrável

- [ ] Rodou no ambiente Vulkan 1.3 identificado por `vulkaninfo`.
- [ ] Testado SSAO OFF e ON, com mesma cena e configurações.
- [ ] Capturas PNG reais, legíveis e sem render em branco.
- [ ] Logs de execução e ausência de VUID registrados.
- [ ] Resize, encerramento, recompilação e inicialização testados.
- [ ] FPS/tempo de frame informados apenas quando realmente medidos.
- [ ] Vídeo de demonstração gravado se exigido pelo professor.

## Empacotamento

- [ ] Código acessível pelo link e/ou ZIP produzido após clone.
- [ ] PDFs e, se exigidos, fontes `.tex`/imagens originais separados.
- [ ] README de instalação com dependências e passos reproduzíveis.
- [ ] README de evidências com commit/OS/GPU/driver/câmera/parâmetros.
- [ ] ZIP testado (`unzip -t`, arquivo abre, conteúdos corretos).
- [ ] Todos os links e arquivos foram revisados; não há dados privados nem credenciais.
