#version 330 core

// in vec2 TexCoord;

out vec4 FragColor;

// uniform sampler2D billboardTexture;

void main() {
    // FragColor = texture(billboardTexture, TexCoord);
    FragColor = vec4(0.0, 0.0, 1.0, 1.0);  // blue color for test
}
