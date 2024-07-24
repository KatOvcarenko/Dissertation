#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in mat4 viewMatrix;

layout (location = 0) out vec4 fragColor;

layout (set = 1, binding = 0) uniform  samplerCube cubeTex;

layout (set = 2, binding  = 0) uniform CameraPos 
{
	vec3	cameraPosition;
};

layout (set = 3, binding  = 0) uniform  sampler2D dudvMap; //Diff descriptor
layout (set = 9, binding  = 0) uniform  sampler2D normalTex;

layout (set = 4, binding  = 0) uniform LightInfo
{
	vec3 lightPos;
	float lightRad;
	vec4 lightCol;
};

layout (set = 8, binding  = 0) uniform  TimeInfo 
{
	float time;
};

layout(push_constant) uniform PushConstantFrag{
	layout(offset = 64) vec4 colour;
}; 

layout (set = 10, binding  = 0) uniform  samplerCube bufferTexDiffC;
layout (set = 11, binding  = 0) uniform  samplerCube bufferTexDepthC;

void main() {
	float density = 0.0005f;
	float grad = 1.5f;
	vec4 positionRelativeToCam = viewMatrix * vec4(inWorldPos,1);
	float distance = length(positionRelativeToCam.xyz);
	float visibility = exp(-pow((distance*density),grad));
	visibility = clamp(visibility, 0.00, 1.00);
	float currentDepth = cameraPosition.y;

	vec4 normalTexCol = texture(normalTex, inTexCoord - time/12.0); //vec2(inTexCoord.x - cos(time/12.0), inTexCoord.y - sin(time/20.0)));//
	vec3 normal = vec3(normalTexCol.r * 2.0 - 1.0, normalTexCol.b, normalTexCol.g * 2.0 - 1.0);
	normal = inNormal - normalize(normal);//;//

	vec3 incident = normalize(lightPos - inWorldPos);
	float lambert = max(0.0, dot(incident, normal)) * 0.5;

	vec3 worldDir = normalize(inWorldPos - cameraPosition);
	vec3 toCamVec = cameraPosition - inWorldPos;
	if(cameraPosition.y < 0)
		toCamVec =  inWorldPos-cameraPosition;
	vec3 viewVec = normalize(toCamVec);
	float refractiveFactor = dot(viewVec, inNormal); // FOR DuDv MAP
	refractiveFactor = pow(refractiveFactor, 3.0);

	fragColor = colour * texture(cubeTex, reflect(worldDir,normal));//texture(dudvMap, inTexCoord);//

	vec3 viewDir = normalize ( cameraPosition - inWorldPos );
	vec3 halfDir = normalize ( incident + viewDir );
	
	float rFactor = max(0.0, dot(halfDir ,normal ));
	float sFactor = pow(rFactor, 100.0 );

	//if(inWorldPos.x>250.0||inWorldPos.x<-250.0||inWorldPos.z>250.0||inWorldPos.z<-250.0)
		//discard;
	vec4 test = texture(bufferTexDiffC,reflect(worldDir,inNormal));
	vec4 sky = texture(cubeTex, reflect(worldDir,normal));
	fragColor.rgb *= vec3(0.5,0.65,0.7); //ambient
	fragColor = mix(fragColor, vec4(colour.rgb,0.2), refractiveFactor); //reflect, refract
	fragColor.rgb += fragColor.rgb * lightCol.rgb * lambert;//* shadow 
	fragColor.rgb += lightCol.rgb * sFactor;
	fragColor.rgb = colour.rgb *test.rgb;
	if(cameraPosition.y > 0.0)
		fragColor.rgb = mix(sky.rgb, fragColor.rgb, visibility);
	else
		fragColor.rgb = mix(vec3(0.0,0.3,0.4), fragColor.rgb, visibility);
}	