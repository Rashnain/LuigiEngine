#version 330 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;
uniform sampler2D heightmap_tex;
uniform float multiplier = 0.5;

out vec2 tex_coord;
out vec3 vtx_position;

void main() {
    tex_coord = uv;
    vtx_position = position;
    vtx_position.y = texture(heightmap_tex, tex_coord).x * multiplier;
    gl_Position = mvp * vec4(vtx_position, 1);
}
