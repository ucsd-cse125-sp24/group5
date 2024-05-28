#version 330 core

in vec4 projectedFragPosition;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

in mat4 finalModel;

in vec4 lightCoordPosn;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragGNormal;
layout (location = 2) out int fragGMask;

uniform mat4 perspective;
uniform mat4 view; // View matrix for converting to canonical coordinates
uniform mat4 model;
uniform vec3 cameraPosition;

uniform vec4 lightDir;
uniform mat4 lightView; // Light's viewing matrix
uniform mat4 lightPerspective; // Light's perspective matrix

uniform int hasDiffuseMap;
uniform sampler2D diffuseTexture;
uniform vec3 diffuseColor;

uniform int hasSpecularMap;
uniform sampler2D specularTexture;
uniform vec3 specularColor;

uniform vec3 emissiveColor;

uniform vec3 ambientColor; // DO NOT USE! ALL AMBIENT COLORS ARE SET TO WHITE!

uniform int hasBumpMap;
uniform sampler2D bumpTexture;

uniform int hasDisplacementMap;
uniform sampler2D displacementTexture;

uniform int hasRoughMap;
uniform sampler2D roughTexture;
uniform vec3 roughColor;

uniform int drawOutline;

uniform sampler2D shadowMap;

uniform vec4 lightPositions[10]; // Positional light positions
uniform int lightActive[10]; // Whether each light is active
uniform vec4 lightColors[10]; // Colors for each light

// Global light color
const vec4 globalLightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

// Copied from wikipedia :) https://en.wikipedia.org/wiki/Smoothstep
float smoothstep(float stepLow, float stepHigh, float lowerBound, float upperBound, float x) {
    x = clamp((x - stepLow) / (stepHigh - stepLow), 0, 1);
    x = x * x * (3.0f - 2.0f * x); // smooth to between 0 and 1
    x = x * upperBound + (1 - x) * lowerBound;
    return x * x * (3.0f - 2.0f * x);
}

vec4 computeDiffuse(vec3 light, vec3 norm, vec4 lightColor, vec4 diffuseColor, float shadow) {
    float nDotL = dot(light, norm);
    if (nDotL > 0.7) {
        nDotL = smoothstep(0.7, 0.71, 0.8, 1, nDotL);
    } else if (nDotL > 0.5) {
        nDotL = smoothstep(0.5, 0.51, 0.6, 0.8, nDotL);
    } else if (nDotL > 0.3) {
        nDotL = smoothstep(0.3, 0.31, 0.5, 0.6, nDotL);
    } else if (nDotL > 0.1) {
        nDotL = smoothstep(0.1, 0.11, 0.45, 0.5, nDotL);
    } else {
        nDotL = 0.4;
    }
    if (shadow > 0) nDotL = 0.4;
    return lightColor * nDotL * diffuseColor;
}

vec4 computeSpecular(vec3 lightDirection, vec3 viewDir, vec3 normal, vec4 lightColor, vec4 specularColor, vec4 shininess) {
    if (dot(normal, lightDirection) <= 0) {
        return vec4(0.0);
    }
    vec3 halfAngle = normalize(lightDirection + viewDir);
    float nDotH = max(0.0f, dot(normal, halfAngle));
    if (nDotH >= 0.99) {
        nDotH = smoothstep(0.99, 1, 0.99, 1, nDotH);
    } else {
        nDotH = 0;
    }
    vec4 dotVec = pow(vec4(nDotH, nDotH, nDotH, 0.0), shininess);
    dotVec.w = 1.0f;
    vec4 color = lightColor * dotVec * specularColor;
    return color;
}

vec4 computeRim(vec3 lightDirection, vec3 viewDir, vec3 normal, vec4 lightColor, vec4 SpecularColor) {
    float rimDot = 1 - dot(viewDir, normal);
    float nDotL = max(0, dot(normal, lightDirection));
    rimDot *= nDotL;
    if (rimDot >= 0.6) {
        rimDot = smoothstep(0.6, 0.61, 0.99, 1, rimDot);
    } else {
        rimDot = 0;
    }
    return rimDot * pow(dot(normal, lightDirection), 0.1) * SpecularColor;
}

float computeShadow(vec3 normal, vec3 lightdir, vec4 position) {
    vec3 projCoords = position.xyz / position.w;

    // OpenGL viewing frustum goes from [-1, 1], transform to [0, 1] range for texture coordinates
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.01 * (1.0 - dot(normal, lightdir)), 0.002);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // PCF filtering to make shadows smoother
    float shadowFactor = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadowFactor += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadowFactor /= 9.0;

    return shadowFactor;
}

void main() {
    vec3 transformedNormal = normalize((transpose(inverse(finalModel)) * vec4(fragNormal, 1)).xyz);
    vec4 position4 = model * vec4(fragPosition, 1.0f);
    vec3 position3 = position4.xyz / position4.w;
    vec3 lightdir = normalize(lightDir).xyz;
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 roughness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec3 viewDir = normalize(cameraPosition - position3);
    float shadow = computeShadow(transformedNormal, lightdir, lightCoordPosn);

    if (hasDiffuseMap != 0) {
        diffuse = texture(diffuseTexture, fragTexcoord);
    } else {
        diffuse = vec4(diffuseColor, 1.0f);
    }

    fragColor = computeDiffuse(lightdir, transformedNormal, globalLightColor, diffuse, shadow);

    if (hasSpecularMap != 0) {
        specular = texture(specularTexture, fragTexcoord);
    } else {
        specular = vec4(specularColor, 1.0f);
    }

    roughness = vec4(roughColor, 1.0f);

    fragColor += (1 - shadow) * clamp(computeSpecular(lightdir, viewDir, transformedNormal, globalLightColor, specular, roughness), 0, 1);
    fragGNormal.xyz = transformedNormal;
    fragGNormal.w = dot(viewDir, transformedNormal);
    fragGMask = drawOutline; // 4th position of fragcolor will denote whether to draw outline
    // Uncomment to enable rim lighting
    // fragColor += clamp(computeRim(lightdir, viewDir, transformedNormal, lightColor, specular), 0, 1);
}
