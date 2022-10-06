#version 440

layout(binding = 0, std140) uniform Ubo {
    mat4 qt_Matrix;
    vec2 gradient;
} ubuf;

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main() {
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) {
        discard;
    }

    float value = texture(tex, uv).r;
    if (value < ubuf.gradient.x || value > ubuf.gradient.y) {
        fragColor = vec4(0, 0, 0, 1);
    } else {
        fragColor = mix(vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), (value - ubuf.gradient.x) / (ubuf.gradient.y - ubuf.gradient.x));
    }
}
