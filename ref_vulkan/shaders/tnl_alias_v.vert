#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;

layout(location = 0) out vec3 oColor;
layout(location = 1) out vec2 oTexCoord;
out gl_PerVertex 
{
    vec4 gl_Position;
};

layout(binding = 1) uniform per_object
{
    mat4 modelviewproj;
};

void main() 
{
    oColor = color;
    oTexCoord = texcoord;
    gl_Position = modelviewproj * vec4(position, 1.);
}
