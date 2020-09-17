#version 440

layout(location = 0) in vec4 out_color;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform buf 
{
    mat4 mvp_matrix;
    vec3 camera_position;
} ubuf;

void main()
{
    frag_color = out_color;
}
