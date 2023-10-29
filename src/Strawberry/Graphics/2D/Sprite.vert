#version 460

vec2 positions[] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
};

void main() {
    gl_Position = vec4(positions[gl_VertexIndex].x, -positions[gl_VertexIndex].y, 0.0, 1.0);
}