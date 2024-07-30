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

layout (set = 3, binding  = 0) uniform  sampler2D dudvMap;
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

layout (set = 12, binding  = 0) uniform  samplerCube bufferTexDiffC2;
layout (set = 13, binding  = 0) uniform  samplerCube bufferTexDepthC2;

const float distortionStrength = 0.02;

void main() {
	float density = 0.0005f;
	float grad = 1.5f;

	//dudv
	vec2 distortion = (texture(dudvMap, vec2(inTexCoord.x*3 + time*0.3, inTexCoord.y*3 + time*0.3)).rg * 2.0 - 1.0) * distortionStrength;

	//vec4 normalTexColdist = texture(normalTex, distortion);

	vec4 positionRelativeToCam = viewMatrix * vec4(inWorldPos,1);
	float distance = length(positionRelativeToCam.xyz);
	float visibility = exp(-pow((distance*density),grad));
	visibility = clamp(visibility, 0.00, 1.00);
	float currentDepth = cameraPosition.y;

	vec4 normalTexCol = texture(normalTex, (inTexCoord + distortion) - 12.0/time);//- time/12.0 
	vec3 normal = vec3(normalTexCol.r * 2.0 - 1.0, normalTexCol.b, normalTexCol.g * 2.0 - 1.0);//* 2.0 - 1.0
	normal = inNormal - normalize(normal*0.5);//;//
	vec3 normals =vec3(0.0,1,0.0);// normal;//
	vec3 incident = normalize(lightPos - inWorldPos);
	float lambert = max(0.0, dot(incident, normal)) * 0.5;

	vec3 worldDir = normalize(inWorldPos - cameraPosition);
	vec3 toCamVec = inWorldPos - cameraPosition;
	if(currentDepth > 0)
		toCamVec =  cameraPosition - inWorldPos;

	vec3 viewVec = normalize(toCamVec);
	float refractiveFactor = dot(viewVec, normals); // FOR DuDv MAP
	refractiveFactor = pow(refractiveFactor, 3.0);

	//fragColor = colour * texture(bufferTexDiffC2, reflect(worldDir,normal));//texture(dudvMap, inTexCoord);//
		//if(currentDepth<0) worldDir = normalize(cameraPosition - inWorldPos);

	vec3 viewDir = normalize ( cameraPosition - inWorldPos );
	vec3 halfDir = normalize ( incident + viewDir );
	
	float rFactor = max(0.0, dot(halfDir ,normal ));
	float sFactor = pow(rFactor, 100.0 );


	vec3 incidentDir = normalize(vec3(worldDir.x,-worldDir.y, -worldDir.z));
	incidentDir.x += distortion.x;
	incidentDir.z += distortion.y;
	vec3 waterNormal = normalize(vec3(normals.x, normals.y, normals.z));
	vec3 reflectedDir = reflect(incidentDir, waterNormal);
	vec4 testReflect = colour * texture(bufferTexDiffC2, reflectedDir);

	incidentDir = normalize(vec3(worldDir.x, worldDir.y, -worldDir.z));
	incidentDir.x += distortion.x;
	incidentDir.z += distortion.y;
	reflectedDir = reflect(incidentDir, waterNormal);
	vec4 testRefract = colour * texture(bufferTexDiffC, reflectedDir);
	
	vec4 sky = texture(cubeTex, reflect(worldDir,normal));

	fragColor.rgb *= vec3(0.5,0.65,0.7); //ambient

	fragColor = mix(testReflect, testRefract, refractiveFactor); //reflect, refract

	fragColor.rgb += fragColor.rgb * lightCol.rgb * lambert;//* shadow 
	fragColor.rgb += lightCol.rgb * sFactor;
	
	if(currentDepth < 0.0)
		fragColor.rgb = mix(sky.rgb, fragColor.rgb, visibility);
	else
		fragColor.rgb = mix(vec3(0.0,0.3,0.4), fragColor.rgb, visibility);
	
	//vec3 normalizedNormal = normalize(inNormal);
    //vec3 color = vec3((normalizedNormal.x * 0.5) + 0.5, (normalizedNormal.y * 0.5) + 0.5, (normalizedNormal.z * 0.5) + 0.5);
	//fragColor = vec4(color, 1.0);//testRefract;//testReflect;//
}	