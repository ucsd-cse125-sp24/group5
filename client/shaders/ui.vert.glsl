#version 330 core

layout (location = 0) in vec2 position;

uniform float aspectRatio;

void main() {
    vec2 pos = vec2(position.x * aspectRatio, position.y);

    gl_Position = vec4(pos, 0, 1);
}
