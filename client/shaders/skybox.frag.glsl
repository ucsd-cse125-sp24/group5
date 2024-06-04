#version 330 core

// Cube map for skybox
uniform samplerCube cubeMap;
// Texture direction
in vec3 texDir;

out vec4 fragColor;

void main() {
    fragColor = texture(cubeMap, texDir);
}
