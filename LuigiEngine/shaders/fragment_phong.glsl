#version 330 core

// Input data (from vertex shader)
in vec2 tex_coord;
in vec3 vtx_normal;
in vec3 vtx_light;
in vec3 vtx_camera;

// Ouput data
out vec3 color;

uniform sampler2D tex;
uniform vec3 light_color;
uniform float k_ambiante;
uniform float k_diffuse;
uniform float k_specular;
uniform int alpha;

void main() {
    vec3 i_ambiante = texture(tex, vec2(tex_coord.x, 1 - tex_coord.y)).xyz * light_color;
    vec3 i_diffuse = i_ambiante * light_color;
    vec3 i_specular = vec3(1, 1, 1) * light_color;

    vec3 ambiante = i_ambiante * k_ambiante;
    vec3 diffuse = i_diffuse * k_diffuse * dot(vtx_normal, vtx_light);
    // TODO [Phong] marche mal
    vec3 reflection = normalize(2 * dot(vtx_normal, vtx_light) * (vtx_normal - vtx_light));
    vec3 specular = i_specular * k_specular * pow(dot(reflection, vtx_camera), alpha);

    color = ambiante + diffuse + specular;
}
