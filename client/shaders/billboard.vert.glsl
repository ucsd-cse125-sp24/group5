#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 aTexCoord; // The texture coordinate

uniform vec3 billboardCenter;
uniform vec2 billboardDimension;
uniform vec3 CameraRight;
uniform vec3 CameraUp;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main() {
    vec3 vertexPosition_worldspace = billboardCenter 
                                    + CameraRight * vertex.x * billboardDimension.x
                                    + CameraUp * vertex.y * billboardDimension.y;
                                    
    // Transform the position to screen space
    gl_Position = projection * view * vec4(vertexPosition_worldspace, 1.0f);
    TexCoord = aTexCoord;
}
