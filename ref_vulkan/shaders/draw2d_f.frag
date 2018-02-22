#version 450

layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform push_const
{
	vec4 color;
};

void main() 
{
    oColor = vec4(texcoord, 0., 1.) * color;
}
