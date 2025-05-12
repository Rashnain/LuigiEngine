#version 330 core

layout(location = 0) in vec3 position; // Vertex position input

uniform mat4 model; // Model transformation matrix

void main() {
    gl_Position = model * vec4(position, 1.0); // Apply model transformation
}
