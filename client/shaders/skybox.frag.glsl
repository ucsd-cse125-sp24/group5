#version 330 core

uniform samplerCube skyboxTex;

in vec3 texDir;

out vec4 fragColor;

void main() {
    fragColor = texture(skyboxTex, texDir);
}
