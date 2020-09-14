#version 440

layout(location = 0) in vec3 out_world_position;
layout(location = 1) in vec3 out_normal;
layout(location = 2) in vec2 out_uv;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform CameraUniform 
{
    mat4 mvp_matrix;
    vec3 camera_position;
} u_camerabuf;

layout(std140, binding = 1) uniform MaterialUniform 
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 shininess;
} u_materialbuf;


layout(binding = 2) uniform sampler2D u_diffuseTexture;


void main()
{
	vec4 diffusePart = texture(u_diffuseTexture, out_uv);
	if(diffusePart.a < 0.0001) // cheap full transparency
		discard;

	// needed as uniform
	vec3 light_pos = vec3(0.0, 30.0, 100.0);
	vec3 light_color = vec3(1.0, 1.0, 1.0);
	float shininess = u_materialbuf.shininess[0];

	// Ambient
    vec3 ambient = u_materialbuf.ambient.xyz * light_color;//ambient_strength * light_color;

    // Diffuse
	vec3 norm = normalize(out_normal);
	vec3 light_dir = normalize(light_pos - out_world_position);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diffusePart.xyz * diff * light_color;


	// Spec
	vec3 view_dir = normalize(u_camerabuf.camera_position - out_world_position);
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
	vec3 specular = u_materialbuf.specular.xyz * spec * light_color;  

    vec3 res_color = ambient + diffuse + specular;
	frag_color = vec4(res_color, 1.0f);
}