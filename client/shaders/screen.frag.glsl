#version 430 core

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D depthTexture;

uniform vec3 cameraPosition;

out vec4 fragColor;

float sobelX[3][3] =
    {{-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};

float sobelY[3][3] =
    {{-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}};

void main() {
    // Camera parameters
    float farPlane = 1000;
    float nearPlane = 0.5;

    vec2 textureSize = textureSize(colorTexture, 0).xy;
    vec3 curColor = texture(colorTexture, texCoord).rgb;
    float curDepth = mix(nearPlane, farPlane, texture(depthTexture, texCoord).x);
    vec3 curNorm = texture(normalTexture, texCoord).xyz;
    float vDotN = texture(normalTexture, texCoord).w;
    vec3 curPos = texture(positionTexture, texCoord).xyz;

    int size = 1;
    float depthDiff = 0;
    float depthDiff2 = 0;
    vec3 normalDiff = vec3(0);
    vec3 normalDiff2 = vec3(0);
    vec3 colorDiff = vec3(0);
    vec3 colorDiff2 = vec3(0);
    float positionDiff = 0 ;

    for (int i = -size; i <= size; i++) {
        for (int j = -size; j <= size; j++) {
            vec2 tc = texCoord + vec2(i, j) / textureSize; // texture coord
            float texDepth = texture(depthTexture, tc).x;
            vec3 texNorm = texture(normalTexture, tc).xyz;
            vec3 texCol = texture(colorTexture, tc).xyz;
            depthDiff += texDepth * sobelX[i + 1][j + 1];
            depthDiff += texDepth * sobelY[i + 1][j + 1];
            normalDiff += texNorm * sobelX[i + 1][j + 1];
            normalDiff += texNorm * sobelY[i + 1][j + 1];
            colorDiff += texCol * sobelX[i + 1][j + 1];
            colorDiff += texCol * sobelY[i + 1][j + 1];

            depthDiff2 -= texDepth * sobelX[i + 1][j + 1];
            depthDiff2 -= texDepth * sobelY[i + 1][j + 1];
            normalDiff2 -= texNorm * sobelX[i + 1][j + 1];
            normalDiff2 -= texNorm * sobelY[i + 1][j + 1];
            colorDiff2 -= texCol * sobelX[i + 1][j + 1];
            colorDiff2 -= texCol * sobelY[i + 1][j + 1];
        }
    }

//    float score = (depthDiff * dot(cameraPosition - curPos, curNorm)) + length(normalDiff) + 0.1 * length(colorDiff);
    float depthScore = max(depthDiff, depthDiff2);
    float normScore = max(length(normalDiff), length(normalDiff2));
    float score = depthScore;
    if (depthScore * vDotN > 0.04 || normScore > 2) {
        fragColor.rgb = vec3(0);
    } else {
        fragColor.rgb = curColor;
    }

}
