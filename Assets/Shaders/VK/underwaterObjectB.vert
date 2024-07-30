#version 460
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inPos;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec4 fragPos;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outWorldPos;
layout (location = 4) out vec4 clipPosition;
layout (location = 5) out mat4 outViewMatrix;

layout (set = 0, binding  = 0) uniform  CameraInfo 
{
	mat4 viewMatrix;
	mat4 projMatrix;
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
};

layout (set = 5, binding  = 0) uniform newViewMat 
{
	mat4 newviewMatrix;
};

void main() {
	mat3 normalMatrix = inverse(transpose(mat3(modelMatrix)));
   	outNormal		= normalMatrix * inNormal;
	outViewMatrix	= newviewMatrix;
	vec4 worldPos	= modelMatrix * vec4(inPos.xyz, 1.0);
	outWorldPos		= worldPos.xyz;
	clipPosition	= worldPos;

	gl_Position	= worldPos;
}