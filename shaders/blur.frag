#version 450
#extension GL_GOOGLE_include_directive : require
#include "comum.glsl"
// PASSO 3: SUAVIZACAO (blur bilateral) da oclusao.
// O SSAO bruto e granulado. Aqui cada pixel vira a media dos vizinhos (5x5), mas so
// entram na media os vizinhos com profundidade parecida: assim a sombra de um objeto
// nao "vaza" para o fundo que esta atras dele.

layout(location = 0) in vec2 inUV;
layout(location = 0) out float outAO;

layout(set = 0, binding = 3) uniform sampler2D gProfundidade;
layout(set = 0, binding = 4) uniform sampler2D aoBruto;

void main() {
    vec2  texel  = 1.0 / vec2(textureSize(aoBruto, 0));  // tamanho de 1 pixel
    float centro = texture(gProfundidade, inUV).r;
    float soma = 0.0, pesos = 0.0;

    for (int y = -2; y <= 2; y++)
    for (int x = -2; x <= 2; x++) {
        vec2  uv   = inUV + vec2(x, y) * texel;
        float dif  = (texture(gProfundidade, uv).r - centro) / max((1.0 - centro) * 0.05, 1e-7);
        float peso = exp(-dif * dif);                     // profundidade diferente -> peso ~0
        soma  += texture(aoBruto, uv).r * peso;
        pesos += peso;
    }
    outAO = soma / pesos;
}
