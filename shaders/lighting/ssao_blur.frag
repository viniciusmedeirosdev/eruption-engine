#version 450

// Bilateral blur over the half-res SSAO target, weighted by DEPTH similarity
// so occlusion never bleeds across depth discontinuities (a prop's AO staying
// off the wall behind it).
//
// Kernel size follows ssaoParams.w (same switch as ssao.frag):
//   0 = engine default 3x3, 1 = academic proposal 5x5.

layout(location = 0) in vec2 inUV;
layout(location = 0) out float outAO;

layout(set = 0, binding = 2)  uniform sampler2D gbufferDepth;
layout(set = 0, binding = 12) uniform sampler2D ssaoRaw;

// Prefixo do LightingUBO (C++) ate' ssaoParams - os offsets batem com ssao.frag.
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
    float sunAngularRadius;
    uint pad1;
    vec4 ambientSky;
    vec4 ambientGround;
    vec4 ssaoParams;        // x=enabled, y=strength, z=radius, w=quality (0=3x3, 1=5x5)
} lights;

// Depth lido por texelFetch: com sampler LINEAR, um texel do AO em meia
// resolucao caia entre 2x2 texels de depth e a MEDIA de primeiro plano e fundo
// enfraquecia justamente o teste de borda que este filtro existe para fazer.
float depthAt(vec2 uv) {
    ivec2 size = textureSize(gbufferDepth, 0);
    return texelFetch(gbufferDepth, clamp(ivec2(uv * vec2(size)), ivec2(0), size - 1), 0).r;
}

void main() {
    // Peso bilateral por PROFUNDIDADE em vez de distancia de mundo: o blur so'
    // precisa saber se dois pixels estao na mesma superficie, e o depth
    // responde isso sem exigir um attachment RGBA16F inteiro no G-buffer.
    float dC = depthAt(inUV);
    vec2 texel = 1.0 / vec2(textureSize(ssaoRaw, 0));
    int r = (lights.ssaoParams.w > 0.5) ? 2 : 1;
    // Tolerancia escala com a profundidade porque o depth e' nao linear.
    float tol = max(dC * 0.0015, 2e-5);

    float total = 0.0;
    float wsum = 0.0;
    for (int y = -r; y <= r; ++y) {
        for (int x = -r; x <= r; ++x) {
            vec2 suv = inUV + vec2(float(x), float(y)) * texel;
            float ao = texture(ssaoRaw, suv).r;
            float dd = (depthAt(suv) - dC) / tol;
            float dw = exp(-dd * dd);
            total += ao * dw;
            wsum += dw;
        }
    }
    outAO = (wsum > 1e-4) ? (total / wsum) : 1.0;
}
