#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in float pointSize;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform push_const
{
    vec2 screen;
};

void main() 
{
    vec2 wpos = vec2(pos.x, 1. - pos.y) * screen;
    float radius = pointSize * .5;
    float dist = length(gl_FragCoord.xy - wpos)/radius;
    if (dist > 1.)
        discard;
    oColor = color;
}
