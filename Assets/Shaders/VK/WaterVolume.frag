/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////

#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;
layout (location = 2) in mat4 viewMatrix;

layout (location = 0) out vec4 fragColor;

layout (set  = 1, binding = 0) uniform  samplerCube cubeTex;

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

layout(push_constant) uniform PushConstantFrag{
	layout(offset = 64) vec4 colour;
}; 

void main() {
	vec4 positionRelativeToCam = viewMatrix * vec4(inWorldPos,1);
	float distance = length(positionRelativeToCam.xyz);
	float visibility = exp(-pow((distance*density),grad));
	visibility = clamp(visibility, 0.00, 1.00);

	float max = inWorldPos.y;
	float gradFactor;
	vec3 fogCol;
	float fogColourMixMin1 = 80.0;
	float fogColourMixMax1 = 160.0;
    if (max > -fogColourMixMin1){
		gradFactor = clamp((max + fogColourMixMin1) / fogColourMixMax1, 0.0, 1.0);
        fogCol = mix(fogColour[1].xyz, fogColour[0].xyz, gradFactor);
	}
    else if (max > -fogColourMixMax1){
		gradFactor = clamp((max + fogColourMixMax1) / fogColourMixMin1, 0.0, 1.0);
        fogCol = mix(fogColour[2].xyz, fogColour[1].xyz, gradFactor);
	}
    else
        fogCol = fogColour[2].xyz;

	vec3 worldDir = normalize(inWorldPos - cameraPosition);
	fragColor = vec4(fogCol, 1.0f) * texture(cubeTex, reflect(worldDir,inNormal));//reflect(worldDir,inNormal));//colour * 


    //fragColor = vec4(fogCol, 1.0f);

	if (max>1.0)
		discard;
}