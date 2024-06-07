#version 330 core

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float alpha;

out vec4 fragColor;

void main() {
    // fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // White color
    fragColor = texture(ourTexture, TexCoord);
    if (alpha <= 0.0001) {

    }
    else if (length(fragColor) >= 0.001)
        fragColor[3] = alpha;

}
