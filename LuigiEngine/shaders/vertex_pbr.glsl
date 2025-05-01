#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

uniform mat4 mvp;
uniform mat4 model;
uniform mat3 rotation;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main() {
    TexCoords = uv;
    WorldPos = vec3(mvp * vec4(position, 1));
    WorldPos = vec3(model * vec4(position, 1));
    Normal = vec3(rotation * normal);
//    Normal = normal;

    gl_Position = vec4(mvp * vec4(position, 1));
}
