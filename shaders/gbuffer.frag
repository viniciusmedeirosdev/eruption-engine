#version 450
#extension GL_GOOGLE_include_directive : require
#include "comum.glsl"
// PASSO 1 (pixels): em vez de iluminar, apenas GUARDA os dados de cada pixel
// visivel no G-buffer: cor (albedo), normal e profundidade (esta e automatica).
// Os passos seguintes (SSAO e imagem final) trabalham so com essas imagens.

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(push_constant) uniform Objeto { mat4 modelo; int terreno; } obj;

layout(set = 0, binding = 6) uniform sampler2D camadasTerreno[10]; // 0..7 = texturas do chao, 8 e 9 = mascaras
layout(set = 1, binding = 0) uniform sampler2D textura;            // textura do material do objeto

layout(location = 0) out vec4 outAlbedo;  // rgb = cor, a = brilho proprio (lava)
layout(location = 1) out vec4 outNormal;  // xyz = normal no mundo

void main() {
    vec4 cor = texture(textura, inUV);
    if (cor.a < 0.5) discard;             // recorta as partes transparentes das folhas

    float brilho = 0.0;
    if (obj.terreno == 1) {
        // O chao mistura 9 texturas (mata, terra, rocha, lava...). Duas imagens de
        // mascara cobrem o mapa inteiro (4000 x 4000 unidades) e cada canal delas diz
        // quanto de cada camada aparece naquele ponto.
        vec2 uvMapa = (inPos.xz + 4.0) / 4000.0;
        vec4 m0 = texture(camadasTerreno[8], uvMapa);
        vec4 m1 = texture(camadasTerreno[9], uvMapa);
        float peso[8] = float[](m0.r, m0.g, m0.b, m0.a, m1.r, m1.g, m1.b, m1.a);

        float pesoBase = max(0.0, 1.0 - (m0.r + m0.g + m0.b + m0.a + m1.r + m1.g + m1.b + m1.a));
        vec3  soma  = cor.rgb * pesoBase;  // a textura base ocupa o que sobrar
        float total = pesoBase;
        for (int i = 0; i < 8; i++) {
            soma  += texture(camadasTerreno[i], inUV).rgb * peso[i];
            total += peso[i];
        }
        cor.rgb = soma / max(total, 0.0001);
        brilho  = peso[6];                 // a camada 6 e a lava: ela emite luz propria
    }

    vec3 n = normalize(inNormal);
    if (!gl_FrontFacing) n = -n;           // folhas sao vistas pelos dois lados

    outAlbedo = vec4(cor.rgb, brilho);
    outNormal = vec4(n, 0.0);
}
