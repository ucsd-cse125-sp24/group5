#version 330 core

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D depthTexture;

uniform vec3 cameraPosition;

out vec4 fragColor;

void main() {
    vec3 sobelX[3];

    sobelX[0] = vec3(-1, 0, 1);
    sobelX[1] = vec3(-2, 0, 2);
    sobelX[2] = vec3(-1, 0,1);

    vec3 sobelY[3];

    sobelY[0] = vec3(-1, -2, -1);
    sobelY[1] = vec3(0, 0, 0);
    sobelY[2] = vec3(1, 2,1);

    // Camera parameters
    float farPlane = 1000;
    float nearPlane = 0.5;

    vec2 textureSize = textureSize(colorTexture, 0).xy;
    vec3 curColor = texture(colorTexture, texCoord).rgb;
    float curDepth = texture(depthTexture, texCoord).x;
    vec3 curNorm = texture(normalTexture, texCoord).xyz;
    float vDotN = texture(normalTexture, texCoord).w;

    int size = 1;
    float depthDiff = 0;
    float depthDiff2 = 0;
    vec3 normalDiff = vec3(0);
    vec3 normalDiff2 = vec3(0);


    for (int i = -size; i <= size; i++) {
        for (int j = -size; j <= size; j++) {
            vec2 tc = texCoord + vec2(i, j) / textureSize;// texture coord
            vec3 texNorm = texture(normalTexture, tc).xyz;
            normalDiff += texNorm * sobelX[i + 1][j + 1];
            normalDiff += texNorm * sobelY[i + 1][j + 1];
            normalDiff2 -= texNorm * sobelX[i + 1][j + 1];
            normalDiff2 -= texNorm * sobelY[i + 1][j + 1];
        }
    }

    vec2 tc1 = texCoord + vec2(-1, -1) / textureSize;
    vec2 tc2 = texCoord + vec2(1, 1) / textureSize;
    depthDiff += pow(texture(depthTexture, tc1).x - texture(depthTexture, tc2).x, 2);
    vec2 tc3 = texCoord + vec2(-1, 1) / textureSize;
    vec2 tc4 = texCoord + vec2(1, -1) / textureSize;
    depthDiff += pow(texture(depthTexture, tc3).x - texture(depthTexture, tc4).x, 2);
    depthDiff = sqrt(depthDiff);
    float normScore = max(length(normalDiff), length(normalDiff2));
    if (depthDiff * vDotN > 0.005 && curDepth > 0.5) {
        fragColor.rgb = (0.05) * curColor;
    } else if (curDepth > 0.8 && normScore - 0.4 * curDepth > 3) {
        fragColor.rgb = vec3(0.05) * curColor;
    } else {
        fragColor.rgb = curColor;
    }

}
