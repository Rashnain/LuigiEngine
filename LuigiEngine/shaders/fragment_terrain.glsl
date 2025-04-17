#version 330 core

// Input data (from vertex shader)
in vec2 tex_coord;
in vec3 vtx_position;

// Ouput data
out vec3 color;

uniform sampler2D snowrock_tex;
uniform sampler2D rock_tex;
uniform sampler2D grass_tex;
uniform float multiplier = 1;

void main() {
    if (vtx_position.y < 0.2 * multiplier)
        color = texture(grass_tex, tex_coord).xyz;
    else if (vtx_position.y < 0.4 * multiplier)
        color = mix(texture(grass_tex, tex_coord), texture(rock_tex, tex_coord), (vtx_position.y-0.2 * multiplier)/(0.2 * multiplier)).xyz;
    else if (vtx_position.y < 0.6 * multiplier)
        color = texture(rock_tex, tex_coord).xyz;
    else if (vtx_position.y < 0.8 * multiplier)
        color = mix(texture(rock_tex, tex_coord), texture(snowrock_tex, tex_coord), (vtx_position.y-0.6 * multiplier)/(0.2 * multiplier)).xyz;
    else
        color = texture(snowrock_tex, tex_coord).xyz;
}