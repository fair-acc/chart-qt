#version 440

layout(binding = 0) uniform Ubo {
	mat4 qt_Matrix;
    vec2 gradient;
    int lineOffset;
} ubuf2;

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main() {
    float value = texture(tex, uv).r;
    if (value < ubuf2.gradient.x || value > ubuf2.gradient.y) {
        fragColor = vec4(0, 0, 0, 1);
    } else {
        fragColor = mix(vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), (value - ubuf2.gradient.x) / (ubuf2.gradient.y - ubuf2.gradient.x));
    }


}
