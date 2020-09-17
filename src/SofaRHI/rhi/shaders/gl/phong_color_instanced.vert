#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 instanceTranslation; // TODO as a transform matrix later

layout(location = 0) out vec3 out_world_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_color;

layout(std140, binding = 0) uniform buf 
{
    mat4 mvp_matrix;
    vec3 camera_position;
} ubuf;

out gl_PerVertex 
{ 
	vec4 gl_Position;
};

void main()
{

    mat4 instanceMatrix = mat4(1, 0, 0, 0,
				               0, 1, 0, 0,
				               0, 0, 1, 0,
				               instanceTranslation.x, instanceTranslation.y, instanceTranslation.z, 1);

    gl_Position = ubuf.mvp_matrix * instanceMatrix * position;
    out_world_position = position.xyz;
    out_normal = normal;
    out_color = color;
}
