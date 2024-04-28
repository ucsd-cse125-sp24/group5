#version 330 core

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

out vec4 fragColor;

void main() {
    fragColor = texture(colorTexture, texCoord);
    vec3 normal = normalize(texture(normalTexture, texCoord).xyz);
    if (normal.x > 0 && texture(colorTexture, texCoord).y > 0 && normal.z > 0) {
//        fragColor = vec4(vec3(texture(depthTexture, texCoord).x), 1);
        fragColor = texture(normalTexture, texCoord);
    }
}
