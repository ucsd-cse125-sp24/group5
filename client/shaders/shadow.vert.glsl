#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord; // Unused but here for compatibility reasons (don't want to redefine VAO's for models)
layout (location = 3) in ivec4 boneidx;
layout (location = 4) in vec4 boneweight;

const int max_bones = 100;
const int max_bone_influence = 4;

uniform bool isAnimated;
uniform mat4 boneTransform[max_bones]; // Up to 100 bones per model

// These uniforms mean different things compared to static.vert.glsl
uniform mat4 perspective; // Light's perspective matrix
uniform mat4 view;        // Light's viewing matrix

uniform mat4 model;       // Object's model transformation matrix

// A simplified version of static.vert.glsl for shadow maps
void main() {
    // Perform vertex transformation
    if (isAnimated) {
        mat4 accumulatorModel = mat4(0);
        vec4 totalPosition = vec4(0);
        for (int i = 0; i < max_bone_influence; i++) {
            if (boneidx[i] == -1) {
                continue;
            } else if (boneidx[i] >= max_bones) {
                // here for debug purposes
                totalPosition = vec4(vertex, 1);
                break;
            }
            vec4 localPos = boneTransform[boneidx[i]] * vec4(vertex, 1);
            accumulatorModel += boneweight[i] * boneTransform[boneidx[i]];
            totalPosition += boneweight[i] * localPos;
        }
        if (boneidx[0] == -1) {
            accumulatorModel = model;
            totalPosition = vec4(vertex, 1);
        }

        gl_Position = perspective * view * model * totalPosition;
    } else {
        gl_Position = perspective * view * model * vec4(vertex, 1);
    }
}
