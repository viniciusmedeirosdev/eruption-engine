#version 450
#extension GL_GOOGLE_include_directive : require
#include "comum.glsl"
// PASSO 4: IMAGEM FINAL. Junta o G-buffer com a oclusao e calcula a iluminacao.
// A oclusao multiplica a LUZ AMBIENTE - e isso que escurece cantos e frestas.

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outCor;

layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;
layout(set = 0, binding = 3) uniform sampler2D gProfundidade;
layout(set = 0, binding = 4) uniform sampler2D aoBruto;
layout(set = 0, binding = 5) uniform sampler2D aoSuave;

void main() {
    vec4  albedo       = texture(gAlbedo, inUV);
    vec3  N            = texture(gNormal, inUV).xyz;
    float profundidade = texture(gProfundidade, inUV).r;

    // Oclusao deste pixel: suavizada ou bruta, conforme o controle da interface.
    float ao = p.blurLigado == 1 ? texture(aoSuave, inUV).r : texture(aoBruto, inUV).r;
    // SSAO desligado, ou pixel a esquerda da divisoria: sem oclusao (para comparar).
    if (p.ssaoLigado == 0 || inUV.x < p.divisoria) ao = 1.0;

    if (profundidade >= 1.0) {
        // Nenhum objeto neste pixel: pinta o ceu (degrade do horizonte ao alto).
        vec3 direcao = normalize(posicaoMundo(inUV, 1.0) - p.posCamera.xyz);
        outCor = vec4(mix(vec3(0.75, 0.82, 0.90), vec3(0.25, 0.45, 0.80), max(direcao.y, 0.0)), 1.0);
    } else {
        // Luz ambiente: azulada vinda do ceu, amarronzada vinda do chao.
        vec3 ambiente = mix(vec3(0.30, 0.25, 0.20), vec3(0.55, 0.65, 0.80), N.y * 0.5 + 0.5) * p.luzAmbiente;
        // Luz direta do sol: depende do angulo entre a superficie e o sol.
        vec3 sol = vec3(1.0, 0.95, 0.85) * max(dot(N, p.dirSol.xyz), 0.0) * p.luzSol;

        vec3 luz = ambiente * ao + sol * mix(1.0, ao, p.aoNaLuzDireta);
        vec3 cor = p.modo == 2 ? vec3(0.8) : albedo.rgb;        // modo "argila": tudo cinza claro
        outCor = vec4(cor * luz + albedo.rgb * albedo.a * 2.0, 1.0); // + brilho proprio da lava

        if (p.modo == 1) outCor = vec4(vec3(ao), 1.0);             // somente a oclusao
        if (p.modo == 3) outCor = vec4(N * 0.5 + 0.5, 1.0);        // normais como cores
        if (p.modo == 4) outCor = vec4(vec3(pow(profundidade, 400.0)), 1.0); // profundidade
    }

    // Linha branca vertical marcando a divisoria da comparacao.
    if (p.divisoria > 0.0 && abs(inUV.x - p.divisoria) < 0.001) outCor = vec4(1.0);
}
