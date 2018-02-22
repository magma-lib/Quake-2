#version 450

layout(location = 0) in vec4 position;

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
    gl_Position = modelviewproj * position;
}
