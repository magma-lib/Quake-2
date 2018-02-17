#version 450

layout(location = 0) in vec2 texcoord0;
layout(location = 1) in vec2 texcoord1;

layout(location = 0) out vec4 oColor;

void main() 
{
    oColor = vec4(texcoord0.xy, 0., 1.);
}
