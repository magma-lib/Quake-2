#version 450

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform push_const
{
	vec4 color;
};

void main() 
{
    oColor = color;
}
