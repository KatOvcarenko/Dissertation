#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 fragPos;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec4 FragPos;

layout (location = 0) out vec4 BufferDiff;
layout (location = 1) out vec4 BufferDepth;

layout (set = 2, binding  = 0) uniform CameraPos 
{
	vec3 cameraPosition;
};

layout (set = 3, binding = 0) uniform  farPlane
{
	float far_plane;
};

layout(push_constant) uniform PushConstantFrag{
	layout(offset = 64) vec4 colour;
}; 

void main() {
	float fragDistance = length(FragPos.xyz - cameraPosition);
    
    fragDistance = fragDistance / far_plane;
    BufferDiff = colour;
    BufferDepth = vec4(fragDistance, fragDistance, fragDistance, 1.0);
    //gl_FragDepth = fragDistance;
}