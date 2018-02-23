#version 450

#define DOT_SIZE 0.25

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 pos;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform push_const
{
    vec2 screen;
	float pointsize;
};

void main() 
{
    vec2 wpos = vec2(pos.x, 1. - pos.y) * screen;
    float radius = pointsize * .5;
    float d = length(gl_FragCoord.xy - wpos)/(radius * DOT_SIZE);
    float a = (d > 1.) ? 0. : color.a;
    oColor = vec4(color.rgb, a);
}
