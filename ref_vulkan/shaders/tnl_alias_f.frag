#version 450

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec4 oColor;

void main() 
{
    oColor = vec4(color, 1.);
}
