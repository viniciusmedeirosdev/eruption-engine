#version 450
#extension GL_GOOGLE_include_directive : require
#include "comum.glsl"
// PASSO 1 (vertices): leva cada vertice do mapa para a tela.

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// Dados que mudam a cada objeto desenhado.
layout(push_constant) uniform Objeto {
    mat4 modelo;   // posiciona/gira/escala o objeto no mundo
    int  terreno;  // 1 = este objeto e o chao do mapa
} obj;

layout(location = 0) out vec3 outPos;     // posicao no mundo
layout(location = 1) out vec3 outNormal;  // normal no mundo
layout(location = 2) out vec2 outUV;      // coordenada de textura

void main() {
    vec4 mundo  = obj.modelo * vec4(inPos, 1.0);
    outPos      = mundo.xyz;
    outNormal   = mat3(obj.modelo) * inNormal;
    outUV       = inUV;
    gl_Position = p.viewProj * mundo;
}
