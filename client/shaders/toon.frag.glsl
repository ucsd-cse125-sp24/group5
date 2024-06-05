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

// point light only
uniform vec3 pointLightPosition;
uniform int danceBomb;          // 1 or 0
uniform int discoLights;        // 1 or 0 

// Whether this material has a diffuse texture
uniform int hasDiffuseMap;

// Multiple textures/diffuse colors to handle different seasons
// Cannot do something like uniform sampler2D texture[ARRAYSIZE] because that has architecture-specific behaviors
uniform sampler2D diffuseTexture0;
uniform sampler2D diffuseTexture1;
uniform sampler2D diffuseTexture2;
uniform sampler2D diffuseTexture3;
uniform vec3 diffuseColor;

uniform int multipleTextures; // 1 or 0 depending on whether this material has multiple diffuse textures
uniform int textureIdx; // Diffuse texture currently selected (out of the possible diffuse textures)

uniform int seasons; // 1 or 0 depending on whether this material is affected by current season
uniform int curSeason; // Current season, value should be between 0 and 3 inclusive
uniform float seasonBlend; // Amount to blend with next aseason

uniform int entityAlternateTexture; // 1 or 0 depending on whether the current entity could swap to alternate diffuse textures
uniform int entitySeasons; // 1 or 0 depending on whether the current entity wants to change color/textures with the seasons

uniform int hasSpecularMap;
uniform sampler2D specularTexture;
uniform vec3 specularColor;

uniform int hasShinyMap;
uniform vec3 shinyColor;
uniform sampler2D shinyTexture;

uniform int drawOutline;

uniform sampler2D shadowMap;

// // Unused positional light stuff
// uniform vec4 lightPositions[10]; // Positional light positions
// uniform int lightActive[10]; // Whether each light is active
// uniform vec4 lightColors[10]; // Colors for each light

// Global light color
const vec4 globalLightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

const vec3 discoColors[8] = vec3[](
    vec3(0.627, 0.125, 0.941), // Purple
    vec3(0.0, 0.7, 0.0),       // Green
    vec3(0.7, 0.7, 0.0),       // Yellow
    vec3(0.7, 0.0, 0.0),       // Red
    vec3(0.0, 0.0, 0.7),       // Blue
    vec3(1.0, 0.5, 0.0),       // Orange
    vec3(0.0, 1.0, 1.0),       // Cyan
    vec3(1.0, 0.0, 0.5)        // Magenta
);

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

vec4 computePointLight(vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 diffuseSampled, vec4 specularSampled, vec3 lightColor) {

    // vec4 lightColor = vec4(1.0, 0.647, 0.0, 1.0); // orange
    // vec4 lightColor = vec4(0.627, 0.125, 0.941, 1.0); // purple
    vec4 lightAmbient = vec4(0.3*lightColor, 1.0);
    vec4 lightDiffuse = vec4(0.5*lightColor, 1.0);
    vec4 lightSpecular = vec4(lightColor, 1.0);

    vec3 lightDirection = normalize(lightPos - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDirection), 0.0); 

    // specular shading
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // shininess = 32.0

    // attenuation
    float constant = 1.0f;
    float linear = 0.19f;
    float quadratic = 0.042f;
    float dist = length(lightPos - fragPos);
    float attenuation = 1.0 / (constant + linear * dist + quadratic * dist*dist);

    // combine results
    vec4 ambient = lightAmbient * diffuseSampled;
    vec4 diffuse = lightDiffuse * diff * diffuseSampled;
    vec4 specular = lightSpecular * spec * specularSampled;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main() {
    vec3 transformedNormal = normalize((transpose(inverse(finalModel)) * vec4(fragNormal, 1)).xyz);
    vec4 position4 = model * vec4(fragPosition, 1.0f);
    vec3 position3 = position4.xyz / position4.w;
    vec3 lightdir = normalize(lightDir).xyz;
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 shiny = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec3 viewDir = normalize(cameraPosition - position3);
    float shadow = computeShadow(transformedNormal, lightdir, lightCoordPosn);

    if (hasDiffuseMap != 0) {
        if (seasons != 0 && entitySeasons != 0) {
            switch (curSeason) {
                case 0:
                    diffuse = mix(texture(diffuseTexture0, fragTexcoord), texture(diffuseTexture1, fragTexcoord), seasonBlend);
                    break;
                case 1:
                    diffuse = mix(texture(diffuseTexture1, fragTexcoord), texture(diffuseTexture2, fragTexcoord), seasonBlend);
                    break;
                case 2:
                    diffuse = mix(texture(diffuseTexture2, fragTexcoord), texture(diffuseTexture3, fragTexcoord), seasonBlend);
                    break;
                case 3:
                    diffuse = mix(texture(diffuseTexture3, fragTexcoord), texture(diffuseTexture0, fragTexcoord), seasonBlend);
                    break;
                default:
                    diffuse = texture(diffuseTexture3, fragTexcoord);
                    break;
            }
        } else if (multipleTextures != 0 && entityAlternateTexture != 0) {
            switch (textureIdx) {
                case 0:
                    diffuse = texture(diffuseTexture0, fragTexcoord);
                    break;
                case 1:
                    diffuse = texture(diffuseTexture1, fragTexcoord);
                    break;
                case 2:
                    diffuse = texture(diffuseTexture2, fragTexcoord);
                    break;
                case 3:
                    diffuse = texture(diffuseTexture3, fragTexcoord);
                    break;
                default:
                    diffuse = texture(diffuseTexture0, fragTexcoord);
                    break;
            }
        } else {
            diffuse = texture(diffuseTexture0, fragTexcoord);
        }
    } else {
        diffuse = vec4(diffuseColor, 1.0f);
    }

    fragColor = computeDiffuse(lightdir, transformedNormal, globalLightColor, diffuse, shadow);

    if (hasSpecularMap != 0) {
        specular = texture(specularTexture, fragTexcoord);
    } else {
        specular = vec4(specularColor, 1.0f);
    }

    if (hasShinyMap != 0) {
        shiny = texture(shinyTexture, fragTexcoord);
    } else {
        shiny = vec4(shinyColor, 1.0f);
    }

    fragColor += (1 - shadow) * clamp(computeSpecular(lightdir, viewDir, transformedNormal, globalLightColor, specular, shiny), 0, 1);
    fragGNormal.xyz = transformedNormal;
    fragGNormal.w = dot(viewDir, transformedNormal);
    fragGMask = drawOutline; // 4th position of fragcolor will denote whether to draw outline
    // Uncomment to enable rim lighting
    // fragColor += clamp(computeRim(lightdir, viewDir, transformedNormal, lightColor, specular), 0, 1);

    if (danceBomb == 0) {
        // tiny point light above the egg
        fragColor += computePointLight(pointLightPosition, transformedNormal, position3, viewDir, diffuse, specular, vec3(0.2, 0.2, 0.2));
    }
    else {
        if (discoLights == 0) {
            // purple light above the unexploded dance bomb
            fragColor += clamp(computePointLight(pointLightPosition, transformedNormal, position3, viewDir, diffuse, specular, discoColors[0]), 0, 1); // purple
        }
        else {
            // more twinkling disco lights around
            fragColor += clamp(computePointLight(pointLightPosition + vec3(5,0,0), transformedNormal, position3, viewDir, diffuse, specular, discoColors[0]), 0, 1);
            fragColor += clamp(computePointLight(pointLightPosition + vec3(0,0,5), transformedNormal, position3, viewDir, diffuse, specular, discoColors[1]), 0, 1);
            fragColor += clamp(computePointLight(pointLightPosition + vec3(-5,0,0), transformedNormal, position3, viewDir, diffuse, specular, discoColors[3]), 0, 1);
            fragColor += clamp(computePointLight(pointLightPosition + vec3(0,0,-5), transformedNormal, position3, viewDir, diffuse, specular, discoColors[7]), 0, 1);
        }
    }
        
}


// const vec4 danceColors[4] = vec4[4](
//     vec4(0.5, 0.0, 0.5, 1.0), // purple
//     vec4(0.0, 1.0, 0.0, 1.0), // green
//     vec4(0.0, 0.0, 1.0, 1.0), // blue
//     vec4(1.0, 0.0, 0.0, 1.0)  // red
// );
