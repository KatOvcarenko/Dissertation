#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;
layout (location = 0) in vec4 fragPos[];
layout (location = 1) in vec2 fragTexCoord[];
layout (location = 2) in vec3 inNormal[];
layout (location = 3) in vec3 inWorldPos[];
layout (location = 4) in vec4 clipPosition[];
layout (location = 5) in mat4 inViewMatrix[];

layout (location = 0) out vec4 geomFragPos;
layout (location = 1) out vec2 geomTexCoord;
layout (location = 2) out vec4 FragPos;
layout (location = 3) out vec3 outNormal;
layout (location = 4) out vec3 outWorldPos;
layout (location = 5) out vec4 clipPositionOut;
layout (location = 6) out mat4 outViewMatrix;

layout (set = 1, binding = 0) uniform CubeMat
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
            geomFragPos = fragPos[i];
            outNormal= inNormal[i];
             outWorldPos=inWorldPos[i];
             outViewMatrix =inViewMatrix[i];
             geomTexCoord = fragTexCoord[i];
             clipPositionOut = clipPosition[i];
            gl_Position = cubeMatrixes[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 