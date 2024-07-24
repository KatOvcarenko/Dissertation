#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inTexCoord;

layout (location = 0) out vec3 texCoord;

layout (set = 0, binding  = 0) uniform  CameraInfo 
{
	mat4 viewMatrix;
	mat4 projMatrix;
};

void main() {
	texCoord = inPosition;
	vec4 worldPos = vec4(inPosition, 1.0);

	gl_Position 	= worldPos;
}
