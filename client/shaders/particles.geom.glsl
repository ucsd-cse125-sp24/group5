#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 vertexColor[];
in mat4 vertexTransform[];

out vec3 particleColor;

uniform mat4 perspective;
uniform mat4 view;

void main() {
    // Get position in canonical space
    vec4 position = view * gl_in[0].gl_Position;

    // quad size
    float size = 0.5;

    // Create quad around original point
    vec4 vertices[4];
    vertices[0] = position + vertexTransform[0] * vec4(-size, -size, 0.0, 0.0);
    vertices[1] = position + vertexTransform[0] * vec4(size, -size, 0.0, 0.0);
    vertices[2] = position + vertexTransform[0] * vec4(-size, size, 0.0, 0.0);
    vertices[3] = position + vertexTransform[0] * vec4(size, size, 0.0, 0.0);

    // create quad for current particle
    for (int i = 0; i < 4; i++) {
        gl_Position = perspective * vertices[i];
        particleColor = vertexColor[0]; // Pass the color to the fragment shader
        EmitVertex();
    }

    EndPrimitive();
}
