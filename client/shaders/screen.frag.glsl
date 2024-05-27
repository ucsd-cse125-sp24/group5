#version 330 core

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform isampler2D maskTexture;
uniform sampler2D depthTexture;

out vec4 fragColor;

void main() {
    vec2 textureSize = textureSize(colorTexture, 0).xy;
    vec3 curColor = texture(colorTexture, texCoord).rgb;
    int curOutline = texture(maskTexture, texCoord).r;
    float curDepth = texture(depthTexture, texCoord).x;
    float vDotN = texture(normalTexture, texCoord).w;

    int size = 1;
    float depthDiff = 0;
    float depthDiff2 = 0;
    float normDiff = 0;

    vec2 tc1 = texCoord + vec2(-1, -1) / textureSize;
    vec2 tc2 = texCoord + vec2(1, 1) / textureSize;
    vec2 tc3 = texCoord + vec2(-1, 1) / textureSize;
    vec2 tc4 = texCoord + vec2(1, -1) / textureSize;
    float normScore = 0;

    // Only draw outline if all surrounding pixels allow outlines
    if (curOutline != 0 && texture(maskTexture, tc1).r != 0 && texture(maskTexture, tc2).r != 0 &&
        texture(maskTexture, tc3).r != 0 && texture(maskTexture, tc4).r != 0) {
        normDiff += length(texture(normalTexture, tc1).xyz - texture(normalTexture, tc2).xyz);
        depthDiff += pow(texture(depthTexture, tc1).x - texture(depthTexture, tc2).x, 2);
        normDiff += length(texture(normalTexture, tc3).xyz - texture(normalTexture, tc4).xyz);
        depthDiff += pow(texture(depthTexture, tc3).x - texture(depthTexture, tc4).x, 2);
        depthDiff = sqrt(depthDiff);
        normScore = length(normDiff);
    }

    if (depthDiff * vDotN > 0.005 && curDepth > 0.5) {
        fragColor.rgb = (0.05) * curColor;
    } else if (curDepth > 0.8 && normScore - 0.4 * curDepth > 3) {
        fragColor.rgb = 0.05 * curColor;
    } else {
        fragColor.rgb = curColor;
    }
}
