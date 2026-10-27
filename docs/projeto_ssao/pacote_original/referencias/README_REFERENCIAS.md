# Referências editáveis

`pipeline_ssao.dot` é uma representação textual do pipeline **da engine real**, escrita para orientar revisão do diagrama anterior. Você pode gerar imagem SVG se tiver Graphviz:

```bash
dot -Tsvg referencias/pipeline_ssao.dot -o referencias/pipeline_ssao.svg
```

Não afirme que este DOT corresponde exatamente ao diagrama da Entrega 2: o PDF anterior modelava 32 amostras, blur 5x5 e `VkRenderPass` explícitos, enquanto a engine utiliza outro fluxo.
