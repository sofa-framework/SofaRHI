#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

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
    gl_Position = ubuf.mvp_matrix * position;
    out_color = color;
}
