#version 450

// Dedicated SSAO pass, rendered at HALF resolution into an R8 target, then
// bilateral-blurred (ssao_blur.frag) and consumed by ambient/directional.
// World-space hemisphere sampling: the world position of every pixel (and of
// every sample) is reconstructed from the G-buffer DEPTH with invViewProj.
// No depth linearization is needed, so the reverse-Z setup stays untouched.
//
// Quality mode (ssaoParams.w):
//   0 = engine default: 12-sample spiral kernel (blur 3x3)
//   1 = academic proposal (Entregas 1-3): 32-sample kernel (blur 5x5)

layout(location = 0) in vec2 inUV;
layout(location = 0) out float outAO;

layout(set = 0, binding = 1) uniform sampler2D gbufferNormal;
layout(set = 0, binding = 2) uniform sampler2D gbufferDepth;

// Must stay a prefix-compatible view of the C++ LightingUBO.
layout(set = 0, binding = 7) uniform LightingUBO {
    vec4 dirLightDir;
    vec4 dirLightColor;
    vec4 pointLights[128];
    vec4 pointColors[128];
    uint numPointLights;
    float giIntensity;
    float giAmbientFloor;
    uint usePbr;
    uint pbrDebugMode;
    float rainIntensity;
    float snowIntensity;
    float temperatureC;
    float pbrLightScale;
    float sunAngularRadius; // era pad0; raio angular da fonte, em radianos
    uint pad1;
    vec4 ambientSky;
    vec4 ambientGround;
    vec4 ssaoParams;        // x=enabled, y=strength, z=radius, w=quality (0=12 amostras, 1=32)
    vec4 indirectParams;
    mat4 viewProj;
    // Cauda do LightingUBO, antes truncada aqui. invViewProj foi ACRESCENTADO
    // no fim da struct em C++, entao nenhum offset antigo mudou.
    vec4 bounceParams;
    vec4 contactParams;
    vec4 skyHorizon;
    mat4 invViewProj;
} lights;


// Posicao de mundo a partir do DEPTH - ver a nota em directional.frag.
vec3 worldFromDepth(vec2 uv, float d) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, d, 1.0);
    vec4 wp = lights.invViewProj * ndc;
    if (abs(wp.w) < 1e-6) return vec3(0.0);
    return wp.xyz / wp.w;
}

// Este passe roda em MEIA resolucao: o centro de cada pixel cai exatamente na
// fronteira entre 2x2 texels do G-buffer. Com sampler LINEAR, depth e normal
// eram a MEDIA de primeiro plano e fundo nas silhuetas - uma posicao "no ar"
// que gerava halo escuro em volta dos objetos. texelFetch le um texel real.
ivec2 gbufferTexel(vec2 uv) {
    ivec2 size = textureSize(gbufferDepth, 0);
    return clamp(ivec2(uv * vec2(size)), ivec2(0), size - 1);
}
float depthAt(ivec2 texel) { return texelFetch(gbufferDepth, texel, 0).r; }

// 12-direction spiral kernel (unit disc, biased toward the centre).
const int KERNEL = 12;
const vec2 kDisc[KERNEL] = vec2[](
    vec2( 0.2310, 0.0000), vec2(-0.2116, 0.2116), vec2( 0.0000,-0.4160), vec2( 0.3536,-0.3536),
    vec2( 0.5590, 0.0000), vec2(-0.4330, 0.4330), vec2( 0.0000, 0.6830), vec2(-0.5303,-0.5303),
    vec2( 0.8080, 0.0000), vec2( 0.6187, 0.6187), vec2( 0.0000,-0.9330), vec2(-0.6894, 0.6894)
);
const int KERNEL_ACADEMIC = 32;

// Ponto i do kernel no disco unitario. Modo academico: espiral de angulo
// aureo com raio sqrt((i+0.5)/N), distribuicao uniforme em area.
vec2 kernelDisc(int i, int count) {
    if (count == KERNEL) return kDisc[i];
    float r = sqrt((float(i) + 0.5) / float(count));
    float a = float(i) * 2.39996323;
    return r * vec2(cos(a), sin(a));
}

float ign(vec2 px) {
    return fract(52.9829189 * fract(dot(px, vec2(0.06711056, 0.00583715))));
}

void main() {
    if (lights.ssaoParams.x < 0.5) { outAO = 1.0; return; }

    ivec2 texel = gbufferTexel(inUV);
    vec2 uv = (vec2(texel) + 0.5) / vec2(textureSize(gbufferDepth, 0));
    vec3 P = worldFromDepth(uv, depthAt(texel));
    vec3 N = texelFetch(gbufferNormal, texel, 0).rgb * 2.0 - 1.0;
    if (any(isnan(P)) || any(isinf(P)) || length(N) < 1e-4) { outAO = 1.0; return; }
    N = normalize(N);

    float radius = max(lights.ssaoParams.z, 0.05);
    int count = (lights.ssaoParams.w > 0.5) ? KERNEL_ACADEMIC : KERNEL;

    // Per-pixel random rotation kills the fixed-kernel banding.
    vec2 px = vec2(texel);
    float a = ign(px) * 6.2831853;
    float ca = cos(a), sa = sin(a);
    mat2 rot = mat2(ca, -sa, sa, ca);

    // Tangent frame around N for hemisphere placement.
    vec3 up = (abs(N.y) < 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);

    float occ = 0.0;
    float used = 0.0;
    for (int i = 0; i < count; ++i) {
        vec2 d = rot * kernelDisc(i, count);
        // Lift the disc into the hemisphere: sample point above the surface.
        float lift = 0.35 + 0.4 * float(i) / float(count);
        vec3 dir = normalize(T * d.x + B * d.y + N * lift);
        vec3 sw = P + dir * radius * (0.25 + 0.75 * float(i + 1) / float(count));

        vec4 clip = lights.viewProj * vec4(sw, 1.0);
        if (clip.w <= 0.0) continue;
        vec2 suv = clip.xy / clip.w * 0.5 + 0.5;
        if (any(lessThan(suv, vec2(0.0))) || any(greaterThan(suv, vec2(1.0)))) continue;

        ivec2 st = gbufferTexel(suv);
        vec2 stUv = (vec2(st) + 0.5) / vec2(textureSize(gbufferDepth, 0));
        vec3 sp = worldFromDepth(stUv, depthAt(st));
        if (any(isnan(sp)) || any(isinf(sp))) continue;

        used += 1.0;
        vec3 dv = sp - P;
        float dist = length(dv);
        if (dist < 0.02 || dist > radius) continue;
        float ndl = dot(dv / dist, N);
        // Surface-plane bias rejects self-occlusion on slopes; range falloff
        // fades distant occluders.
        if (ndl > 0.15) {
            occ += (ndl - 0.15) * (1.0 - dist / radius);
        }
    }
    if (used > 0.5) occ /= used;

    outAO = clamp(1.0 - occ * lights.ssaoParams.y * 2.2, 0.0, 1.0);
}
