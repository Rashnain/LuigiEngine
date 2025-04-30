#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 rotation;
uniform vec3 light_pos;
uniform vec3 camera_pos;

out vec2 tex_coord;
out vec3 vtx_normal;
out vec4 vtx_position;
out vec3 vtx_light;
out vec3 vtx_camera;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

void main() {
    tex_coord = uv;
    vtx_normal = vec3(rotation * normal);
    vtx_position = vec4(model * vec4(position, 1));
    vtx_light = normalize(light_pos - vec3(vtx_position));
    vtx_camera = normalize(camera_pos - vec3(vtx_position));
    gl_Position = vec4(mvp * vec4(position, 1));
}
