#version 460
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "PerlinNoise.glslh"

layout (location = 0) in vec4 inPos;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outWorldPos;
layout (location = 2) out vec2 outTexCoord;
layout (location = 3) out mat4 outViewMatrix;

layout (set = 0, binding  = 0) uniform  CameraInfo 
{
	mat4 viewMatrix;
	mat4 projMatrix;
};

layout (set = 5, binding = 0) uniform waveInfos1{
	vec2 dir;
	float step;
	float wLen;
	float speed;
} WaveA;

layout (set = 6, binding = 0) uniform waveInfos2{
	vec2 dir;
	float step;
	float wLen;
	float speed;
} WaveB;

layout (set = 7, binding = 0) uniform waveInfos3{
	vec2 dir;
	float step;
	float wLen;
	float speed;
} WaveC;

layout (set = 8, binding  = 0) uniform  TimeInfo 
{
	float time;
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
};

const float PI = 3.14;
const float tiling = 200;//16.0;

vec3 tangent ;
vec3 binormal;

vec3 gerstner(vec2 dir, float steepness, float wavelength, vec3 vertex, float speed) {
	
	float waveNum = 2 * PI/wavelength;
	float g = sqrt(9.8/waveNum);
	float f = waveNum * dot(dir, vertex.xz) - g * (time/speed);
	float antiloop = steepness/waveNum;

	tangent += vec3(-dir.x * dir.x *(steepness * sin(f)), dir.x * (steepness * cos(f)), -dir.x * dir.y * (steepness *sin(f)));
	binormal += vec3(-dir.x * dir.y *(steepness * sin(f)), dir.y * (steepness * cos(f)), -dir.y * dir.y * (steepness *sin(f)));

	float dir_x = dir.x * (antiloop * cos(f));
	float dir_z = dir.y * (antiloop * cos(f));
	float dir_y = antiloop * sin(f);
	
	return vec3(dir_x, dir_y, dir_z);
}

void main() {
	tangent = vec3(1, 0, 0);
	binormal = vec3(0, 0, 1);

	vec3 wavePosition = inPos.xyz;
	
	wavePosition += gerstner(normalize(vec2(0.5,0.2)), cos(time)*0.1, 70, inPos.xyz, 3.0);	
	wavePosition += gerstner(normalize(WaveA.dir), WaveA.step, WaveA.wLen, inPos.xyz, WaveA.speed);	
	wavePosition += gerstner(normalize(WaveB.dir), WaveB.step, WaveB.wLen, inPos.xyz, WaveB.speed);	
	wavePosition += gerstner(normalize(WaveC.dir), WaveC.step, WaveC.wLen, inPos.xyz, WaveC.speed);
	wavePosition.y += PerlinNoise(vec2(wavePosition.x,wavePosition.z), time*1.2, 240.0, 15.0);
	//wavePosition.y += PerlinNoise(vec2(wavePosition.x,wavePosition.z), time*0.5, 10.0, 1.5);

	vec3 waveNormal = normalize(cross(binormal, tangent));
	mat3 normalMatrix = inverse(transpose(mat3(modelMatrix)));

   	outNormal	= normalMatrix * waveNormal; 
	outViewMatrix = viewMatrix;
	vec4 worldPos	= modelMatrix * vec4(wavePosition.xyz, 1.0);
	outWorldPos		= worldPos.xyz;
	outTexCoord		= vec2(wavePosition.x/2.0 + 0.5, wavePosition.z/2.0 + 0.5) / tiling;
	
    gl_Position	= projMatrix * viewMatrix * worldPos;
}