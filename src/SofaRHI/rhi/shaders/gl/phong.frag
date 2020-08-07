#version 440

layout(location = 0) in vec3 out_world_position;
layout(location = 1) in vec3 out_normal;
layout(location = 2) in vec2 out_uv;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform buf 
{
    mat4 projection_mv_matrix;
    vec3 camera_position;
} ubuf;

void main()
{
	// needed as uniform
	vec3 object_color = vec3(1.0, 0.0, 0.0);
	vec3 light_pos = vec3(0.0, 0.0, 10.0);
	vec3 light_color = vec3(1.0, 1.0, 1.0);
	float ambient_strength = 0.1;
	float specular_strength = 1.0;
	float shininess = 32.0;

	// Ambient
    vec3 ambient = ambient_strength * light_color;

    // Diffuse
	vec3 norm = normalize(out_normal);
	vec3 light_dir = normalize(light_pos - out_world_position);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * light_color;

	// Spec
	vec3 view_dir = normalize(ubuf.camera_position - out_world_position);
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;  

    vec3 res_color = object_color * (ambient + diffuse + specular);
    frag_color = vec4(res_color, 1.0f);
    //frag_color = vec4(out_position.z, out_position.z, out_position.z, 1.0f);
}
