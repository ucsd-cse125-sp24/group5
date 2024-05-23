#version 330 core

in vec2 TexCoord;

uniform sampler2D ourTexture;

out vec4 fragColor;

void main() {
    // fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // White color
    fragColor = texture(ourTexture, TexCoord);

}
