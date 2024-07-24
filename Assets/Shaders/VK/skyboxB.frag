#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 FragPos;
layout (location = 1) in vec3 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BufferDepth;

layout (set  = 1, binding = 0) uniform  samplerCube skyTex;

void main() {
	vec3 col[6];
	col[0]=vec3(1,0,0);
	col[1]=vec3(-1,0,0);
	col[2]=vec3(0,1,0);
	col[3]=vec3(0,-1,0);
	col[4]=vec3(0,0,1);
	col[5]=vec3(0,0,-1);

	//fragColor = vec4((texCoords + 1.0) * 0.5, 1.0);
   fragColor 	= texture(skyTex, texCoords);
   BufferDepth = vec4(0, 0, 0, 1.0);
}