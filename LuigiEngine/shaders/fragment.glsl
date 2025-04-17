#version 330 core

// Input data (from vertex shader)
in vec2 tex_coord;

// Ouput data
out vec3 color;

uniform sampler2D tex;

void main() {
    color = texture(tex, vec2(tex_coord.x, 1 - tex_coord.y)).xyz;
}
