#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (location = 0) in vec3 outPos[];

layout (location = 0) out vec4 FragPos;
layout (location = 1) out vec3 outviewDir;

layout (set = 2, binding = 0) uniform CubeMat
{
	mat4 cubeMatrixes[6];
};

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            outviewDir = outPos[i];
            gl_Position = cubeMatrixes[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 