#version 330 core

in vec3 particleColor;

out vec4 fragColor;

void main() {
    fragColor = vec4(particleColor, 1);
}
