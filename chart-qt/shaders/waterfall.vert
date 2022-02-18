#version 440

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 uv_in;

layout(binding = 0, std140) uniform Ubo {
    mat4 qt_Matrix;
    vec2 gradient;
    int lineOffset;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec2 uv;

void main() {
    uv = (uv_in * vec2(1, -1) + vec2(0, ubuf.lineOffset / 500.)) * vec2(1, 1);
    gl_Position = ubuf.qt_Matrix * vec4(vertex, 0, 1);
}
