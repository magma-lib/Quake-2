#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord0;
layout(location = 2) in vec2 texcoord1;

layout(location = 0) out vec2 oTexCoord0;
layout(location = 1) out vec2 oTexCoord1;
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
    oTexCoord0 = texcoord0;
    oTexCoord1 = texcoord1;
    gl_Position = modelviewproj * vec4(position, 1.);
}
