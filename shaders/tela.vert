#version 450
// Usado pelos passos 2, 3 e 4: desenha UM triangulo gigante que cobre a tela
// inteira, sem precisar de buffer de vertices. Assim o fragment shader roda
// uma vez para cada pixel da tela.
layout(location = 0) out vec2 outUV;  // coordenada do pixel na tela, de 0 a 1

void main() {
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
}
