#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float aspectRatio;
uniform mat3 trans;

void main() {
    vec3 pos = vec3(position.x * aspectRatio, position.y, 1.0); // avoid distortion due to screen dimension
    pos = trans * pos;  // scale, translation

    gl_Position = vec4(pos.x, pos.y, 0, 1);
    TexCoord = aTexCoord;
}
