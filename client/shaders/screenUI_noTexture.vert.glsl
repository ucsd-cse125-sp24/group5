#version 330 core

layout (location = 0) in vec2 position;

uniform float aspectRatio;

void main() {
    gl_Position = vec4(position.x * aspectRatio, position.y, 0, 1);
}
