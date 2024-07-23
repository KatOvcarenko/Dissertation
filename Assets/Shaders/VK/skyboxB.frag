#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 FragPos;
layout (location = 1) in vec3 viewDir;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BufferDepth;

layout (set  = 1, binding = 0) uniform  samplerCube skyTex;

void main() {
   fragColor 	= texture(skyTex, viewDir);
    
   BufferDepth = vec4(0, 0, 0, 1.0);
}