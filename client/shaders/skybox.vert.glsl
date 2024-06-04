#version 330 core

uniform mat4 perspective;
uniform mat4 view;

layout (location = 0) in vec3 vertex;
// Texture direction
out vec3 texDir;

void main() {
    gl_Position = perspective * view * vec4(vertex, 1.0f);
    texDir = vertex;
}
