#version 330 core

// Texture direction
in vec3 texDir;

// Cube map for skybox
uniform samplerCube cubeMap;

out vec4 fragColor;

void main() {
    fragColor = texture(cubeMap, texDir);
}
