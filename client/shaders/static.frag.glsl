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

/**
 * Compute diffuse lighting with a directional light, assumes lightDirection is normalized
 */
vec4 computeDiffuse(vec3 lightDirection, vec3 normal, vec4 lightColor, vec4 diffuseColor) {
    float nDotL = max(0.0f, dot(lightDirection, normal));
    vec4 color = lightColor * nDotL * diffuseColor;
    return color;
}

/**
 *
 */
vec4 computeSpecular(vec3 lightDirection, vec4 lightColor, vec4 specularColor, vec4 roughness) {
    vec3 eyeDir = normalize(cameraPosition - fragPosition);
    vec3 sum = lightDirection + eyeDir;
    vec3 halfAngle = normalize(sum);
    vec4 nDotH = pow(max(0.0f, dot(normal, halfAngle)), roughness);
    vec4 color = lightColor * nDotH * specularColor;
    return color;
}


void main() {
    vec3 transformedNormal = (inverse(transpose(model)) * vec4(fragNormal, 1)).xyz;
    vec3 lightdir = normalize(lightPosition).xyz;
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (hasDiffuseMap != 0) {
        vec4 texColor = texture(diffuseTexture, fragTexcoord);
        diffuse = mult * texColor;
    } else {
        diffuse = vec4(mult * diffuseColor, 1);
    }
    fragColor += computeDiffuse(lighdir, transformedNormal, lightColor, diffuse);
    if (hasSpecularMap != 0) {
        vec4 texColor = texture(specularTexture, fragTexcoord);

    } else {

    }
    fragColor += vec4(0.1 * diffuseColor, 1.0f);
}
