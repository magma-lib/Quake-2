#version 450

layout(location = 0) in vec3 position;

out gl_PerVertex 
{
    vec4 gl_Position;
};

layout(binding = 0) uniform perspective_transform
{
    mat4 modelviewproj;
};

void main() 
{
    gl_Position = modelviewproj * vec4(position, 1.);
}
