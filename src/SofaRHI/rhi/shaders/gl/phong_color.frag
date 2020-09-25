#version 440

layout(location = 0) in vec3 out_world_position;
layout(location = 1) in vec3 out_normal;
layout(location = 2) in vec4 out_color;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform buf 
{
    mat4 mvp_matrix;
    vec3 camera_position;
} u_camerabuf;

void main()
{
	// needed as uniform
	vec3 object_color = out_color.xyz;
	vec3 light_pos = u_camerabuf.camera_position; //vec3(0.0, 30.0, 100.0);
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
	vec3 view_dir = normalize(u_camerabuf.camera_position - out_world_position);
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
	vec3 specular = specular_strength * spec * light_color;  

    vec3 res_color = object_color * (ambient + diffuse + specular);
    frag_color = vec4(res_color, out_color.a);
    //frag_color = vec4(out_position.z, out_position.z, out_position.z, 1.0f);
}
