#version 330 core

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D depthTexture;

out vec4 fragColor;

void main() {
    // Camera parameters
    float farPlane = 1000;
    float nearPlane = 0.5;

    vec2 textureSize   = textureSize(colorTexture, 0).xy;
    vec3 curColor = texture(colorTexture, texCoord).rgb;
    float curDepth = mix(nearPlane, farPlane, texture(depthTexture, texCoord).x);
    vec3 curNorm = texture(normalTexture, texCoord).xyz;
    vec3 curPos = texture(positionTexture, texCoord).xyz;

    int normKernelSize = 3; // Kernel size
    float depthDiff = 0;
    float normalDiff = 0;
    float colorDiff = 0;
    float positionDiff = 0 ;

    for (int i = -normKernelSize; i <= normKernelSize; i++) {
        for (int j = -normKernelSize; j <= normKernelSize; j++) {
            vec2 tmpCoord = texCoord + (vec2(i, j) / textureSize);
            vec3 texColor = texture(colorTexture, tmpCoord).rgb;
            vec3 texNorm = texture(normalTexture, tmpCoord).xyz;
            vec3 tmpColorDiff = curColor - texColor;
            vec3 tmpNormDiff = curNorm - texNorm;
            colorDiff += abs(tmpColorDiff.x) + abs(tmpColorDiff.y) + abs(tmpColorDiff.z);
            normalDiff += abs(tmpNormDiff.x) + abs(tmpNormDiff.y) + abs(tmpNormDiff.z);
        }
    }

    int depthKernelSize = 2;
    for (int i = -depthKernelSize; i <= depthKernelSize; i++) {
        for (int j = -depthKernelSize; j <= depthKernelSize; j++) {
            vec2 tmpCoord = texCoord + (vec2(i, j) / textureSize);
            float texDepth = mix(nearPlane, farPlane, texture(depthTexture, tmpCoord).x);
            depthDiff = max(depthDiff, abs(curDepth - texDepth));
            vec3 texPos = texture(positionTexture, tmpCoord).xyz;
            vec3 tmpPosDiff_v = curPos - texPos;
            float tmpPosDiff = abs(tmpPosDiff_v.x) + abs(tmpPosDiff_v.y) + abs(tmpPosDiff_v.z);
            positionDiff = max(positionDiff, tmpPosDiff);
        }
    }

    colorDiff /= ((2 * normKernelSize + 1) * (2 * normKernelSize + 1));
    normalDiff /= ((2 * normKernelSize + 1) * (2 * normKernelSize + 1));
    vec3 colorModifier = vec3(0, 0, 0);
    if (normalDiff > 0.6) {
        fragColor.rgb = mix(curColor.rgb, colorModifier, clamp(smoothstep(0, 1, normalDiff), 0.0, 1.0));
    } else if ((curDepth / farPlane) > 0.5 && depthDiff > 5 && ((normalDiff > 0.1 && positionDiff > 0.4) || colorDiff > 0.01)) {
        fragColor.rgb = mix(curColor.rgb, colorModifier, smoothstep(5, 7, depthDiff));
    } else {
        fragColor = vec4(curColor, 1);
    }
}
