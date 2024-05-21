#version 330 core

layout (points) in;
layout (triangles, max_vertices=3) out;

uniform mat4 perspective;
uniform mat4 view;

void main() {
    vec4 position = view * gl_in[0].gl_Position;
    position.xyz /= position.w;
    position.w = 1;

    gl_Position = perspective * (position + vec4(-0.1, 0, 0, 0));
    EmitVertex();
    gl_Position = perspective * (position + vec4(0.1, 0, 0, 0));
    // TODO: finish this
}
