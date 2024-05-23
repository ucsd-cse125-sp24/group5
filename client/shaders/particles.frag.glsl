#version 330 core

in vec4 particleColor;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragGNormal;
layout (location = 2) out int fragGMask;


void main() {
    fragColor = particleColor;
    fragGNormal = vec4(0, 0, 1, 1);
    fragGMask = 0;
}
