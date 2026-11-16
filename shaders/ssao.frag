#version 450
#extension GL_GOOGLE_include_directive : require
#include "comum.glsl"
// PASSO 2: OCLUSAO DE AMBIENTE (SSAO - Screen Space Ambient Occlusion).
// Ideia: cantos, frestas e a base dos objetos recebem menos luz ambiente porque a
// geometria vizinha "tampa" parte do ceu. Para cada pixel, testamos varios pontos
// ao redor dele (no hemisferio da normal) e contamos quantos encontram geometria
// por perto. Saida: 1 = totalmente aberto (claro), 0 = totalmente ocluido (escuro).

layout(location = 0) in vec2 inUV;
layout(location = 0) out float outAO;

layout(set = 0, binding = 2) uniform sampler2D gNormal;
layout(set = 0, binding = 3) uniform sampler2D gProfundidade;

// Numero pseudoaleatorio por pixel: gira o conjunto de amostras em cada pixel para
// trocar "faixas" visiveis por um ruido fino (que o passo 3 suaviza).
float ruido(vec2 pixel) {
    return fract(52.9829189 * fract(dot(pixel, vec2(0.06711056, 0.00583715))));
}

void main() {
    float profundidade = texture(gProfundidade, inUV).r;
    if (profundidade >= 1.0) { outAO = 1.0; return; }       // ceu: nada a ocluir

    vec3 P = posicaoMundo(inUV, profundidade);               // ponto da superficie neste pixel
    vec3 N = texture(gNormal, inUV).xyz;                     // para onde a superficie aponta

    // Dois eixos perpendiculares a normal (T e B) para montar o hemisferio de amostragem.
    vec3 T = normalize(cross(abs(N.y) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0), N));
    vec3 B = cross(N, T);

    float giro = ruido(gl_FragCoord.xy) * 6.2831853;
    float oclusao = 0.0;

    for (int i = 0; i < p.amostras; i++) {
        // Distribui as amostras em espiral (angulo aureo) dentro do hemisferio.
        float t      = (float(i) + 0.5) / float(p.amostras);
        float angulo = float(i) * 2.3999632 + giro;
        float r      = sqrt(t);
        vec3  dir    = T * (r * cos(angulo)) + B * (r * sin(angulo)) + N * sqrt(1.0 - t);
        vec3  amostra = P + dir * p.raio * mix(0.15, 1.0, t);  // ponto de teste perto de P

        // Projeta o ponto de teste na tela para descobrir qual pixel o cobre...
        vec4 clip = p.viewProj * vec4(amostra, 1.0);
        vec2 uv   = clip.xy / clip.w * 0.5 + 0.5;
        if (clip.w <= 0.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) continue;

        // ...e le no G-buffer a superficie REAL que aparece naquele pixel.
        vec3  vizinho   = posicaoMundo(uv, texture(gProfundidade, uv).r);
        vec3  v         = vizinho - P;
        float distancia = length(v);
        if (distancia < 0.001 || distancia > p.raio) continue; // longe demais: nao conta

        // Se a superficie vizinha esta ACIMA do plano do pixel, ela bloqueia luz.
        // Quanto mais alta e mais proxima, maior a oclusao. O bias ignora vizinhos
        // quase no mesmo plano (evita manchas em superficies lisas e inclinadas).
        float altura = dot(v / distancia, N);
        oclusao += max(altura - p.bias, 0.0) * (1.0 - distancia / p.raio);
    }

    float ao = clamp(1.0 - oclusao / float(p.amostras) * p.intensidade * 4.0, 0.0, 1.0);
    outAO = pow(ao, p.contraste);
}
