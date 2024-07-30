/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////

#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 positionRelativeToCam;
layout (location = 1) in vec3 inPos;
layout (location = 3) in vec3 viewDir;

layout (location = 0) out vec4 fragColor;

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

   fragColor 	= texture(skyTex, viewDir);

    if(cameraPosition.y > 0.001){	
		fragColor 	= texture(skyTexUnder, viewDir);
	}
}