#version 460
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inPos;
layout (location = 2) in vec2 texCoord;

layout (location = 0) out vec4 fragPos;
layout (location = 1) out vec2 fragTexCoord;

layout (set = 0, binding  = 0) uniform  CameraInfo 
{
	mat4 viewMatrix;
	mat4 projMatrix;
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
};

void main() {
	gl_Position	= modelMatrix * vec4(inPos.xyz, 1.0);
	//vec4 worldPos = modelMatrix * vec4(inPos.xyz, 1.0);
	//fragPos = worldPos;
	//gl_Position	=  projMatrix * viewMatrix * worldPos;

	//gl_Position	= inPos;
}
