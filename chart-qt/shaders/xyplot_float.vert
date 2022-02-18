#version 440
layout(location = 0) in float vx;
layout(location = 1) in float vy;
layout(binding = 0) uniform Ubo {
	mat4 qt_Matrix;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    gl_Position = ubuf.qt_Matrix * vec4(vx, vy, 0, 1);
}
