#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec2 oPos;
layout(location = 1) out float oPointSize;
layout(location = 2) out vec4 oColor;

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
    float h;
	float pointsize;
};

void main() 
{
    gl_Position = viewproj * position;
    float inv_wclip = 1./gl_Position.w; 
    gl_PointSize = h * pointsize * inv_wclip; // scale with distance

    oPos = gl_Position.xy * inv_wclip * 0.5 + 0.5;
    oPointSize = gl_PointSize; 
    oColor = color;
}
