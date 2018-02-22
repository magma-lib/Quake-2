#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 oTexCoord;
out gl_PerVertex 
{
    vec4 gl_Position;
};

layout(binding = 0) uniform per_frame
{
    mat4 modelviewproj;
    mat4 ortho;
};

void main() 
{
    oTexCoord = texcoord;
    gl_Position = ortho * position;
}
