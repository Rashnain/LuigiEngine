#version 330 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

// Values that stay constant for the whole mesh.
uniform mat4 mvp;
uniform mat4 model;
uniform mat3 rotation;
uniform vec3 light;
uniform vec3 camera;

out vec2 tex_coord;
out vec3 vtx_normal;
out vec4 vtx_position;
out vec3 vtx_light;
out vec3 vtx_camera;

void main() {
    tex_coord = uv;
    vtx_normal = vec3(rotation * normal);
    vtx_position = vec4(model * vec4(position, 1));
    vtx_light = normalize(light - vec3(vtx_position));
    vtx_camera = normalize(camera - vec3(vtx_position));
    gl_Position = vec4(mvp * vec4(position, 1));
}
