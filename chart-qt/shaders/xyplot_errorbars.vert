#version 440
layout(location = 0) in vec2 pos;

layout(binding = 0) uniform Ubo {
	mat4 qt_Matrix;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    gl_Position = ubuf.qt_Matrix * vec4(pos, 0, 1);
}
