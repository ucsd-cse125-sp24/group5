#version 330 core

layout (location = 0) in vec2 position;

uniform float aspectRatio;
uniform float scale;

void main() {

    vec2 pos = position * scale;   // emotive scale
    pos.x *= aspectRatio;          // screen height/width

    gl_Position = vec4(pos, 0, 1);
}
