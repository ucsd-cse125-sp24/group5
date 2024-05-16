#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in ivec4 boneidx;
layout (location = 4) in vec4 boneweight;

out vec4 projectedFragPosition;
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexcoord;
out mat4 finalModel;
out vec4 lightCoordPosn;

const int max_bones = 100;
const int max_bone_influence = 4;

uniform bool isAnimated;
uniform mat4 boneTransform[max_bones]; // Up to 100 bones per model

uniform mat4 perspective; // Camera perspective projection matrix
uniform mat4 view;        // Camera viewing matrix
uniform mat4 model;       // Model transformation matrix

uniform mat4 lightView;   // Light's viewing matrix
uniform mat4 lightPerspective; // Light's perspective matrix

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

        projectedFragPosition = perspective * view * model * totalPosition;
        finalModel = model * accumulatorModel;
        gl_Position = projectedFragPosition;

        fragPosition = vertex;
        fragNormal = normal;
        fragTexcoord = texcoord;

        lightCoordPosn = lightPerspective * lightView * model * totalPosition;
    } else {
        projectedFragPosition = perspective * view * model * vec4(vertex, 1);
        gl_Position = projectedFragPosition;

        // Pass interpolated values to fragment shader
        finalModel = model;
        fragPosition = vertex;
        fragNormal = normal; // Transform normal according to model transformation
        fragTexcoord = texcoord;

        lightCoordPosn = lightPerspective * lightView * model * vec4(vertex, 1);
    }
}
