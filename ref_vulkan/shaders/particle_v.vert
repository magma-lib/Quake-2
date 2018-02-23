#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec2 oPos;
out gl_PerVertex 
{
    vec4 gl_Position;
    float gl_PointSize;
};

layout(binding = 0) uniform per_frame
{
    mat4 viewproj;
};

layout(push_constant) uniform push_const
{
    vec2 screen;
	float pointsize;
};

void main() 
{
    gl_Position = viewproj * position;
    gl_PointSize = pointsize;

    oPos = gl_Position.xy/gl_Position.w * 0.5 + 0.5;
    oColor = color;
}
