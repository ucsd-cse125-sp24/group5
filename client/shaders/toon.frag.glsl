#version 330 core

in vec4 projectedFragPosition;
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

in mat4 finalModel;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragGNormal;

uniform mat4 perspective;
uniform mat4 view; // View matrix for converting to canonical coordinates
uniform mat4 model;
uniform vec3 cameraPosition;

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

uniform sampler2DShadow shadowMap;

//uniform vec4 globalLightColor;
//uniform vec4 globalLightPosition;

const vec4 lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
// 0 in homogenous coordinate for directional light
const vec4 lightPosition = vec4(1.0f, 1.0f, 0.0f, 0.0f);

// Copied from wikipedia :) https://en.wikipedia.org/wiki/Smoothstep
float smoothstep(float stepLow, float stepHigh, float lowerBound, float upperBound, float x) {
    // Scale, and clamp x to range
    x = clamp((x - stepLow) / (stepHigh - stepLow), 0, 1);
    x = x * x * (3.0f - 2.0f * x); // smooth to between 0 and 1
    x = x * upperBound + (1 - x) * lowerBound;
    return x * x * (3.0f - 2.0f * x);
}

/**
 * Compute diffuse lighting with a directional light, assumes lightDirection is normalized
 */
vec4 computeDiffuse(vec3 light, vec3 norm, vec4 lightColor, vec4 diffuseColor) {
    float nDotL = dot(light, norm);
    if (nDotL > 0.8) {
        nDotL = smoothstep(0.8, 0.9, 0.8, 1, nDotL);
    } else if (nDotL > 0.6) {
        nDotL = smoothstep(0.6, 0.63, 0.6, 0.8, nDotL);
    } else if (nDotL > 0.4) {
        nDotL = smoothstep(0.4, 0.43, 0.5, 0.6, nDotL);
    } else if (nDotL > 0.2) {
        nDotL = smoothstep(0.2, 0.23, 0.4, 0.5, nDotL);
    } else {
        nDotL = 0.4;
    }
    return lightColor * nDotL * diffuseColor;
}

/**
 * Compute specular reflection, assumes lightDirection, viewDir, normal are all normalized.
 */
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

/**
 * Compute rim lighting at object edges
 */
vec4 computeRim(vec3 lightDirection, vec3 viewDir, vec3 normal, vec4 lightColor, vec4 SpecularColor) {
    float rimDot = 1 - dot(viewDir, normal);
    // Comment out nDotL stuff for fresnel lighting
    float nDotL = max(0, dot(normal, lightDirection));
    rimDot *= nDotL;
    // Comment out below to disable quantizing rim lighting
    if (rimDot >= 0.6) {
        rimDot = smoothstep(0.6, 0.61, 0.99, 1, rimDot);
    } else {
        rimDot = 0;
    }
    return rimDot * pow(dot(normal, lightDirection), 0.1) * SpecularColor;
}


void main() {
    vec3 transformedNormal = normalize((transpose(inverse(finalModel)) * vec4(fragNormal, 1)).xyz);
    vec4 position4 = model * vec4(fragPosition, 1.0f);
    vec3 position3 = position4.xyz / position4.w;
    vec3 lightdir = normalize(lightPosition).xyz;
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 roughness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec3 viewDir = normalize(cameraPosition - position3);

    // Use albedo texture if one exists, else use default material properties
    if (hasDiffuseMap != 0) {
        diffuse = texture(diffuseTexture, fragTexcoord);
    } else {
        diffuse = vec4(diffuseColor, 1.0f);
    }

    fragColor = computeDiffuse(lightdir, transformedNormal, lightColor, diffuse);

    // Use specular map if one exists, else use default material properties
    if (hasSpecularMap != 0) {
        specular = texture(specularTexture, fragTexcoord);
    } else {
        specular = vec4(specularColor, 1.0f);
    }

    // Use rough map if one exists, else use material properties
    if (hasRoughMap != 0) {
        roughness = texture(roughTexture, fragTexcoord);
    } else {
        roughness = vec4(roughColor, 1.0f);
    }

    fragColor += clamp(computeSpecular(lightdir, viewDir, transformedNormal, lightColor, specular, roughness), 0, 1);
    fragGNormal.xyz = transformedNormal;
    fragGNormal.w = dot(viewDir, transformedNormal);
    // Comment out to disable rim lighting
//    fragColor += clamp(computeRim(lightdir, viewDir, transformedNormal, lightColor, specular), 0, 1);

}
