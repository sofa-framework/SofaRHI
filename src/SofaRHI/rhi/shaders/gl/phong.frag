#version 440

#define MAX_MATERIALS 10

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

layout(location = 0) in vec3 out_world_position;
layout(location = 1) in vec3 out_normal;
layout(location = 2) in vec2 out_uv;
layout(location = 3) in flat int out_materialID;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform CameraUniform 
{
    mat4 mvp_matrix;
    vec3 camera_position;
} u_camerabuf;

layout(std140, binding = 1) uniform MaterialUniform 
{
    Material materials[MAX_MATERIALS];
} u_materialbuf;

void main()
{
	Material material = u_materialbuf.materials[out_materialID];

	// needed as uniform
	vec3 object_color = vec3(1.0, 0.0, 0.0);
	vec3 light_pos = vec3(0.0, 0.0, 10.0);
	vec3 light_color = vec3(1.0, 1.0, 1.0);
	float ambient_strength = 0.1;
	float specular_strength = 1.0;
	float shininess = 32.0;

	// Ambient
    vec3 ambient = material.ambient * light_color;//ambient_strength * light_color;

    // Diffuse
	vec3 norm = normalize(out_normal);
	vec3 light_dir = normalize(light_pos - out_world_position);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = material.diffuse * diff * light_color;

	// Spec
	vec3 view_dir = normalize(u_camerabuf.camera_position - out_world_position);
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
	vec3 specular = material.specular * spec * light_color;  

    vec3 res_color = ambient + diffuse + specular;
    frag_color = vec4(res_color, 1.0f);
   // frag_color = vec4(material.diffuse, 1.0) ;//vec4(out_position.z, out_position.z, out_position.z, 1.0f);}
}