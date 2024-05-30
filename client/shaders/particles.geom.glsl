#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 vertexColor[];
in mat4 vertexTransform[]; // Will encode particle's rotation and translation

out vec4 particleColor;

uniform mat4 perspective;
uniform mat4 view;
uniform float particleSize;

void main() {
    // Get position in canonical space
    mat4 rotation = mat4(mat3(vertexTransform[0]));
    vec4 position = view * vertexTransform[0] * vec4(0, 0, 0, 1);

    // Create quad around original point
    vec4 vertices[4];
    vertices[0] = position + rotation * vec4(-particleSize, -particleSize, 0.0, 0);
    vertices[1] = position + rotation * vec4(particleSize, -particleSize, 0.0, 0);
    vertices[2] = position + rotation * vec4(-particleSize, particleSize, 0.0, 0);
    vertices[3] = position + rotation * vec4(particleSize, particleSize, 0.0, 0);

    // create quad for current particle
    for (int i = 0; i < 4; i++) {
        gl_Position = perspective * vertices[i];
        particleColor = vertexColor[0]; // Pass the color to the fragment shader
        EmitVertex();
    }

    EndPrimitive();
}
