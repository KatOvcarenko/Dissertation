#version 450
#extension GL_ARB_separate_shader_objects   : enable
#extension GL_ARB_shading_language_420pack  : enable
//#extension GL_GOOGLE_include_directive	    : enable

//#include "table.glslh"

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

layout (set = 4, binding  = 0) uniform LightInfo
{
	vec3 lightPos;
	float lightRad;
	vec4 lightCol;
};

layout(push_constant) uniform PushConstantFrag{
	layout(offset = 64) vec4 colour;
};
//layout(push_constant) uniform PushConstantFrag{
//	layout(offset = 128) vec4 colour;
//};

vec3 RGBtoHSV(vec3 rgb){
    float H,S,V;
    float maxRGB, minRGB;
    float inR = rgb.r;
    float inG = rgb.g;
    float inB = rgb.b;

    maxRGB = max(max(inR, inG),inB);
    minRGB = min(min(inR, inG),inB);

    V = maxRGB;
    S = (maxRGB - minRGB)/maxRGB;
    if (V==0)
        S=0;

    if ((maxRGB-minRGB) == 0.0) {
        H = 0.0;
    } else if (maxRGB == inR) {
        H = (inG - inB) / (maxRGB-minRGB);
    } else if (maxRGB == inG) {
        H = 2.0 + (inB - inR) / (maxRGB-minRGB);
    } else {
        H = 4.0 + (inR - inG) / (maxRGB-minRGB);
    }

    H*=60;
    if (H<0)
        H+=360.0;

    return vec3(H,S,V);
}

vec3 HSVtoRGB(vec3 hsv){
    float H = hsv.x;

    float C = hsv.y * hsv.z;
    float X = C * (1 - abs(mod(H/60.0, 2.0) - 1.0));
    float m = hsv.z - C;
    vec3 outRGB;

    if ((H>=0.0) && (H<60))			outRGB = vec3(C,X,0);
    else if ((H>=60.0) && (H<120))	outRGB = vec3(X,C,0);
    else if ((H>=120.0) && (H<180)) outRGB = vec3(0,C,X);
    else if ((H>=180.0) && (H<240)) outRGB = vec3(0,X,C);
    else if ((H>=240.0) && (H<300)) outRGB = vec3(X,0,C);
    else outRGB = vec3(C,0,X);

    outRGB.r += m;     
    outRGB.g += m;     
    outRGB.b += m;

    return outRGB;
}

float maxDepthColor(vec3 baseCol){
	vec3 hsv = RGBtoHSV(baseCol.rgb);
    float H = hsv.x;
    float S = hsv.y;
    float V = hsv.z;
    //float normDepth;
    float maxDepth;
    if (baseCol.r == baseCol.g && baseCol.g == baseCol.b && (V == 1 || V ==0)) //white or black
		H = 180;
    
	if ((H>0) && (H<=30))			maxDepth = -25.0	* 1.0;  
    else if((H>30) && (H<=60))      maxDepth = -60.0	* 1.0; 
    else if((H>60) && (H<=120))     maxDepth = -125.0	* 1.0; 
    else if((H>120) && (H<=180))    maxDepth = -200.0	* 1.0;   
    else if((H>180) && (H<=240))    maxDepth = -135.0	* 1.0;   
    else if((H>240) && (H<=300))    maxDepth = -70.0	* 1.0;
    else                            maxDepth = -10.0	* 1.0;

	return maxDepth;
    //normDepth = 1-clamp(currentDepth/maxDepth, 0.0, 1.0);
}

vec4 underwaterColour(vec3 baseCol, float currentDepth){
	float normDepth;
    vec3 hsv = RGBtoHSV(baseCol.rgb);
    //int x = int(hsv.x);
    //int d = (-1) * int(currentDepth);
	float maxDepth = maxDepthColor(baseCol);//lookupTable(x, d);//
    normDepth = 1-clamp(currentDepth/maxDepth, 0.0, 1.0);
	
	vec3 colourDepth ;//= HSVtoRGB(hsv);
	if (baseCol.r != baseCol.g || baseCol.g != baseCol.b){
		hsv.z = clamp(1 + normDepth - (hsv.z*0.99), 0.0, 1.0);
		colourDepth = HSVtoRGB(hsv);
		//if (hsv.z==0)
			//colourDepth.b = 1-clamp(colourDepth.b + 0.1, 0.0, 0.1);
		return vec4(colourDepth, 1.0);
	}
	
    colourDepth = HSVtoRGB(hsv);

	if (hsv.z==0) //black
		colourDepth.b = clamp(1.0-colourDepth.b - normDepth, 0.0, 0.01);
	else if (hsv.y ==0 && hsv.z==1){ //white
		colourDepth.rg = 1-clamp(colourDepth.rg - normDepth, 0.0, 1.0);
	}

	return vec4(colourDepth, 1.0);
}

void main() {
	vec4 positionRelativeToCam = viewMatrix * vec4(inWorldPos,1);
	float distance = length(positionRelativeToCam.xyz);
	float visibility = exp(-pow((distance*density),grad));
	visibility = clamp(visibility, 0.00, 1.00);

	float currentDepth = inWorldPos.y;
	float gradFactor;
	vec3 fogCol;
	vec3 midCol = mix(fogColour[2].xyz, fogColour[1].xyz, 0.5);

    if (currentDepth > -fogColourMixMin){
		gradFactor = clamp((currentDepth + fogColourMixMin) / fogColourMixMax, 0.0, 1.0);
        fogCol = mix(fogColour[1].xyz, fogColour[0].xyz, gradFactor);
	}
    else if (currentDepth > -fogColourMixMax){
		gradFactor = clamp((currentDepth + fogColourMixMax) / fogColourMixMin, 0.0, 1.0);
        fogCol = mix(midCol, fogColour[1].xyz, gradFactor);
	}
    else if (currentDepth < -fogColourMixMax*2){
		gradFactor = clamp((currentDepth + fogColourMixMax + (fogColourMixMax*2)) / fogColourMixMax, 0.0, 1.0);
		vec3 depthcol = mix(fogColour[1].xyz, midCol, 0.7);
        fogCol = mix(depthcol, midCol, gradFactor);
	}
    else
        fogCol = midCol;
	
	vec3 incident = normalize(lightPos - inWorldPos);
	float lambert = max(0.0, dot(incident, inNormal)) * 0.9;

	vec4 c = underwaterColour(colour.rgb, currentDepth);

	fragColor.rgb = c.rgb * 0.8f;//c.rgb * vec3(0.8, 0.8, 1.0); //
	fragColor.rgb += c.rgb * lightCol.rgb * lambert;//* shadow 
    
    if(currentDepth<2.0)
        fragColor = mix(vec4(fogCol,1.0), c, visibility);
	fragColor.a = 1.0;
}