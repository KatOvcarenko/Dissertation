#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 FragPos;
layout (location = 1) in vec3 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BufferDepth;

layout (set  = 1, binding = 0) uniform  samplerCube skyTex;
layout (set  = 4, binding = 0) uniform  samplerCube skyTexUnder;

layout (set = 2, binding  = 0) uniform CameraPos 
{
	vec3	cameraPosition;
};

layout (set = 3, binding  = 0) uniform FogInfo
{
	vec4	fogColour[3];
	float	fogColourMixMin;
	float	fogColourMixMax;

	float grad;
	float density;
};

void main() {

   fragColor 	= texture(skyTex, texCoords);
   if(cameraPosition.y < 0.001){	
		fragColor 	= texture(skyTexUnder, texCoords);
	}

   BufferDepth = vec4(0, 0, 0, 1.0);
}