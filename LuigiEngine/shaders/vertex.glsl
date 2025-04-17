#version 330 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;

out vec2 tex_coord;

void main() {
    tex_coord = uv;
    gl_Position = mvp * vec4(position, 1);
}
