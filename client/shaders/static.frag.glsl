#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexcoord;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;
uniform vec3 cameraPosition;

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

out vec4 fragColor;

const vec4 lightColor = {1.0f, 1.0f, 1.0f, 1.0f};
// 0 in homogenous coordinate for directional light
const vec4 lightPosition = {1.0f, 1.0f, 0.0f, 0.0f};

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
    if (nDotL > 0.6) {
        nDotL = smoothstep(0.6, 0.63, 0.7, 1, nDotL);
    } else if (nDotL > 0.4) {
        nDotL = smoothstep(0.4, 0.43, 0.5, 0.7, nDotL);
    } else if (nDotL > 0.2) {
        nDotL = smoothstep(0.2, 0.23, 0.4, 0.5, nDotL);
    } else {
        nDotL = 0.4;
    }
    return lightColor * nDotL * diffuseColor;
}

/**
 *
 */
vec4 computeSpecular(vec3 lightDirection, vec3 position, vec3 normal, vec4 lightColor, vec4 specularColor, vec4 roughness) {
    if (dot(normal, lightDirection) <= 0) {
        return vec4(0.0);
    }
    vec3 eyeDir = normalize(cameraPosition - position);
    vec3 halfAngle = normalize(lightDirection + eyeDir);
    float nDotH = max(0.0f, dot(normal, halfAngle));
    if (nDotH >= 0.99) {
        nDotH = smoothstep(0.99, 1, 0.99, 1, nDotH);
    } else {
        nDotH = 0;
    }
    vec4 dotVec = pow(vec4(nDotH, nDotH, nDotH, 0.0), roughness);
    dotVec.w = 1.0f;
    vec4 color = lightColor * dotVec * specularColor;
    return color;
}

vec4 computeRim(vec3 lightDirection, vec3 position, vec3 normal, vec4 lightColor, vec4 SpecularColor, vec4 roughness) {
    vec3 eyeDir = normalize(cameraPosition - position);
    return (1 - dot(eyeDir, normal)) * pow(dot(normal, lightDirection), 0.1) * SpecularColor;
}


void main() {
    vec3 transformedNormal = normalize((inverse(transpose(model)) * vec4(fragNormal, 1)).xyz);
    vec4 position4 = model * vec4(fragPosition, 1.0f);
    vec3 position3 = position4.xyz / position4.w;
    vec3 lightdir = normalize(lightPosition).xyz;
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 roughness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
//    fragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    if (hasDiffuseMap != 0) {
        diffuse = texture(diffuseTexture, fragTexcoord);
    } else {
        diffuse = vec4(diffuseColor, 1.0f);
    }
    fragColor = computeDiffuse(lightdir, transformedNormal, lightColor, diffuse);
    if (hasSpecularMap != 0) {
        specular = texture(specularTexture, fragTexcoord);
    } else {
        specular = vec4(specularColor, 1.0f);
    }
    if (hasRoughMap != 0) {
        roughness = texture(roughTexture, fragTexcoord);
    } else {
        roughness = vec4(roughColor, 1.0f);
    }
    fragColor += clamp(computeSpecular(lightdir, position3, transformedNormal, lightColor, specular, roughness), 0, 1);
//    fragColor += clamp(computeRim(lightdir, position3, transformedNormal, lightColor, specular, roughness), 0, 1);
}
