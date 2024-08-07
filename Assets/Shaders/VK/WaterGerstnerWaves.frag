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

	vec4 positionRelativeToCam = viewMatrix * vec4(inWorldPos,1);
	float distance = length(positionRelativeToCam.xyz);
	float visibility = exp(-pow((distance*density),grad));
	visibility = clamp(visibility, 0.00, 1.00);
	float currentDepth = cameraPosition.y;

	vec4 normalTexCol = texture(normalTex, (inTexCoord - distortion));// - 12.0/time  time/12.0
	vec3 normal = vec3(normalTexCol.r * 2.0 - 1.0, normalTexCol.b, normalTexCol.g * 2.0 - 1.0);//* 2.0 - 1.0
	normal = normalize(inNormal) - normalize(normal);//*0.3;//
	
	vec3 normals = vec3(0.0, 1.0, 0.0);

	vec3 incident = normalize(lightPos - inWorldPos);
	float lambert = max(0.0, dot(incident, normal)) * 0.5;

	vec3 worldDir = normalize(inWorldPos - cameraPosition);

	vec3 toCamVec = inWorldPos - cameraPosition;
	if(currentDepth > 0)
		toCamVec =  cameraPosition - inWorldPos;
	vec3 viewVec = normalize(toCamVec);

	//fresnel effect
	float refractiveFactor1 = dot(viewVec, normal);
	refractiveFactor1 = pow(refractiveFactor1, 0.05);
	refractiveFactor1 = clamp((refractiveFactor1 * 0.5) + 0.5, 0.01, 1.0);

	float refractiveFactor2 = dot(viewVec, normals);
	refractiveFactor2 = pow(refractiveFactor2, 0.05);

	float refractiveFactor = mix( refractiveFactor2, refractiveFactor1, 0.5);

	vec3 incidentDir = normalize(vec3(worldDir.x,-worldDir.y, -worldDir.z));
	incidentDir.x += distortion.x;
	incidentDir.z += distortion.y;
	vec3 waterNormal = normalize(vec3(normals.x, normals.y, normals.z));
	vec3 reflectedDir = reflect(incidentDir, waterNormal);
	vec4 testReflect = colour * texture(bufferTexDiffC2, reflectedDir);

	incidentDir = normalize(vec3(worldDir.xy, -worldDir.z));
	incidentDir.x += distortion.x;
	incidentDir.z += distortion.y;
	vec3 refractedDir = reflect(incidentDir, waterNormal);
	vec4 testRefract = colour * texture(bufferTexDiffC, refractedDir);
	
	vec4 sky = texture(cubeTex, reflect(vec3(worldDir.x,-worldDir.y, worldDir.z), normal*0.5));

	fragColor = mix(testReflect, testRefract, refractiveFactor); //reflect, refract
	
    vec3 ambient = vec3(0.5,0.65,0.7) * lightCol.rgb;
    
    vec3 norm		= normalize(normal);
    vec3 lightDir	= normalize(lightPos - inWorldPos);
    float diff		= max(dot(norm, lightDir), 0.0);
    vec3 diffuse	= diff * vec3(0.5,0.5,0.5);//lightCol.rgb;
    
	vec3 viewDir	= normalize ( cameraPosition - inWorldPos );
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec		= pow(max(dot(viewDir, reflectDir), 0.0), 50);
    vec3 specular	= spec * lightCol.rgb;
    
    vec3 result = (ambient + diffuse) * fragColor.rgb;
	result += specular;

	if(currentDepth > 0.0)
		result = mix(vec3(0.3f, 0.6f, 0.8f), result, visibility);
	else
		result = mix(sky.rgb, result, visibility);

	fragColor = vec4(result, 1.0);

	//vec3 normalizedNormal = normalize(normal);
    //vec3 color = (normalizedNormal * 0.5) + 0.5;
	//fragColor = vec4(color, 1.0);// testReflect;//testRefract;//
}	