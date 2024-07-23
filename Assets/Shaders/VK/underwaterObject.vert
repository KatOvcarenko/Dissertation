#version 460
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inPos;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;
layout (location = 2) out mat4 outViewMatrix;
//layout (location = 3) out float dist;

layout (set = 0, binding  = 0) uniform  CameraInfo 
{
	mat4 viewMatrix;
	mat4 projMatrix;
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
	//mat4 nviewMatrix;
};

void main() {
	mat3 normalMatrix = inverse(transpose(mat3(modelMatrix)));
   	outNormal	= normalMatrix * inNormal;
	outViewMatrix = viewMatrix;
	vec4 worldPos	= modelMatrix * vec4(inPos.xyz, 1.0);
	outWorldPos		= worldPos.xyz;

   gl_Position	= projMatrix * viewMatrix * worldPos;
}