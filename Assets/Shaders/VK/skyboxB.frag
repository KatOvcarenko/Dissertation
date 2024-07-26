#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 FragPos;
layout (location = 1) in vec3 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BufferDepth;

layout (set  = 1, binding = 0) uniform  samplerCube skyTex;

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

	//fragColor = vec4((texCoords + 1.0) * 0.5, 1.0);
   fragColor 	= texture(skyTex, texCoords);

//   if(FragPos.y<0){
//		float currentDepth = FragPos.y;
//	float gradFactor;
//	vec3 fogCol;
//	vec3 midCol = mix(fogColour[2].xyz, fogColour[1].xyz, 0.5);
//
//    if (currentDepth > -fogColourMixMin){
//		gradFactor = clamp((currentDepth + fogColourMixMin) / fogColourMixMax, 0.0, 1.0);
//        fogCol = mix(fogColour[1].xyz, fogColour[0].xyz, gradFactor);
//	}
//    else if (currentDepth > -fogColourMixMax){
//		gradFactor = clamp((currentDepth + fogColourMixMax) / fogColourMixMin, 0.0, 1.0);
//        fogCol = mix(midCol, fogColour[1].xyz, gradFactor);
//	}
//    else if (currentDepth < -fogColourMixMax*2){
//		gradFactor = clamp((currentDepth + fogColourMixMax + (fogColourMixMax*2)) / fogColourMixMax, 0.0, 1.0);
//		vec3 depthcol = mix(fogColour[1].xyz, midCol, 0.7);
//        fogCol = mix(depthcol, midCol, gradFactor);
//	}
//    else
//        fogCol = midCol;
//		vec4 c = texture(skyTex, texCoords);
//        fragColor = mix(vec4(fogCol,1.0), c, visibility);
//   }

   BufferDepth = vec4(0, 0, 0, 1.0);
}