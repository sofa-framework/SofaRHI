#version 440

layout(location = 0) in vec3 out_world_position;
layout(location = 1) in vec3 out_normal;
layout(location = 2) in vec2 out_uv;

layout(location = 0) out vec4 frag_color;

void main()
{
	//needed as uniform
	// vec3 object_color = vec3(0.5, 0.5, 0.5);
	// vec3 light_pos = vec3(0.0, 0.0, 0.0);
	// vec3 light_color = vec3(1.0, 0.0, 0.0);
	// float ambient_strength = 0.1;

	// // Ambient
 //    vec3 ambient = ambient_strength * light_color;

 //    // Diffuse
	// vec3 norm = normalize(normal);
	// vec3 light_dir = normalize(light_pos - world_position);
	// float diff = max(dot(norm, light_dir), 0.0);
	// vec3 diffuse = diff * light_color;

    vec3 res_color = out_normal;//object_color * (ambient + diffuse);
    frag_color = vec4(res_color, 1.0f);
}
