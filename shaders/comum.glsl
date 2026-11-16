// comum.glsl - trecho incluido por todos os shaders.
// Bloco de parametros enviado pelo C++ a cada quadro (struct Parametros em main.cpp).
// A ordem e os tipos precisam ser IDENTICOS nos dois lados.
layout(set = 0, binding = 0) uniform Parametros {
    mat4  viewProj;       // mundo -> tela (projecao * camera)
    mat4  invViewProj;    // tela -> mundo (usada para reconstruir a posicao a partir da profundidade)
    vec4  posCamera;      // posicao da camera no mundo
    vec4  dirSol;         // direcao que aponta PARA o sol
    float raio;           // SSAO: alcance da busca por oclusao, em unidades de mundo
    float intensidade;    // SSAO: forca do escurecimento
    float bias;           // SSAO: tolerancia que evita a superficie sombrear a si mesma
    float contraste;      // SSAO: expoente aplicado ao resultado
    int   amostras;       // SSAO: quantos pontos vizinhos sao testados por pixel
    int   ssaoLigado;     // 1 = aplica a oclusao na imagem final
    int   blurLigado;     // 1 = usa a oclusao suavizada; 0 = usa a oclusao bruta (com ruido)
    int   modo;           // 0 final, 1 somente AO, 2 argila, 3 normais, 4 profundidade
    float divisoria;      // posicao (0..1) da linha: a esquerda dela a imagem fica SEM oclusao
    float luzAmbiente;    // intensidade da luz ambiente (e nela que a oclusao atua)
    float luzSol;         // intensidade da luz direta do sol
    float aoNaLuzDireta;  // 0..1: quanto a oclusao tambem escurece a luz do sol (exagero didatico)
} p;

// Reconstroi a posicao de mundo de um pixel usando apenas sua coordenada de tela (0..1)
// e o valor guardado no buffer de profundidade.
vec3 posicaoMundo(vec2 uv, float profundidade) {
    vec4 m = p.invViewProj * vec4(uv * 2.0 - 1.0, profundidade, 1.0);
    return m.xyz / m.w;
}
