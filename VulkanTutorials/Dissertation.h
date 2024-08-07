#pragma once
#include "VulkanTutorial.h"
#include <vulkan/vulkan.hpp>

namespace NCL::Rendering::Vulkan {
	class Dissertation : public VulkanTutorial
	{
	public:
		Dissertation(Window& window);
		~Dissertation() {}
	protected:
		void RenderFrame(float dt) override;
		void DrawObj(const Vector3& translation = Vector3(0.0f, 0.0f, 0.0f), const Vector3& scale = Vector3(1.0f, 1.0f, 1.0f),
			const Vector4& colour = Vector4(0.0f, 0.0f, 0.0f, 1.0f), const int DS = 0, const int& pipeline = 0, const int& meshName = 0, 
			const float& angle = 0, const Vector3& axis = Vector3(1, 0, 0));
		void DrawWaves(const int pipeline = 0);
		void CreteUniforms();
		void ShaderLoader(); 
		void ColourCheck(const int numOfP = 0, const int DS = 0);
		void DrawSkyBox(const int num = 0, const int DS = 2);
		void CreateSSBOBuffers(uint32_t width, uint32_t height);
		void PipelinesBuilder();
		void FillBufferCubeUpSideDown();
		void FillBufferCube();
		void UpdateDescriptors();
		void CreteDescriptorSets();
		void InvertCamera();
		void UpdateUniformsBufferRefract();
		void UpdateUniformsBufferReflect();
		//void CreateImageFromData();		
		void CreateDescriptors();
		vk::UniqueImageView GenImageView(VulkanTexture* tex, vk::ImageAspectFlags aspects);

		int RENDERAREA;
		int rows;		
		vk::UniqueImageView		cubeFaceView[4];
		PerspectiveCamera		BCamera;

		//std::vector<std::vector<std::string>> readCSV(const std::string& filePath);
		//std::vector<std::vector<std::string>> lookup_table;
		std::vector<Vector3> colours;

		UniqueVulkanShader		skyboxShader;
		UniqueVulkanShader		skyboxShaderB;
		UniqueVulkanShader		objectShader;
		UniqueVulkanShader		objectShaderB;
		UniqueVulkanShader		groundShader;
		UniqueVulkanShader		waveShader;
		UniqueVulkanShader		ssboThreeDBufferShader;
		//UniqueVulkanCompute		lookupTableShader;

		UniqueVulkanTexture		cubeTex;
		UniqueVulkanTexture		cubeTex2;
		UniqueVulkanTexture		cubeTexs[2];
		//UniqueVulkanTexture		lookupTableTex;
		UniqueVulkanTexture		rainbowTex; 
		UniqueVulkanTexture		dudvmapTex;
		UniqueVulkanTexture		waterNormalTex;
		UniqueVulkanTexture		ssboTexDiffuse;
		UniqueVulkanTexture		ssboTexDepth;
		UniqueVulkanTexture		ssboTexDiffuse2;
		UniqueVulkanTexture		ssboTexDepth2;

		VulkanBuffer			camPosUniform;
		VulkanBuffer			camPosUniformSky;
		VulkanBuffer			newViewcameraBufferUniform;
		VulkanBuffer			fogUniform;
		VulkanBuffer			lightUniform; 
		VulkanBuffer			waterNormalTexUniform;
		VulkanBuffer			timeUniform;
		VulkanBuffer			waveUniform[3];
		VulkanBuffer			farPlaneUniform;
		VulkanBuffer			ViewMatUniformRefract;
		VulkanBuffer			ViewMatUniformReflect;
		VulkanBuffer			clippingPlaneUniform[2];
		//VulkanBuffer			lookupTableUniform;

		vk::UniqueDescriptorSet waveDescriptor[3];
		vk::UniqueDescriptorSet timeDescriptor;
		vk::UniqueDescriptorSet	cubemapDescriptor;
		vk::UniqueDescriptorSet	cubemapDescriptor2;
		vk::UniqueDescriptorSet farPlaneDescriptor;
		vk::UniqueDescriptorSet	ViewMatRefractDescriptor;
		vk::UniqueDescriptorSet	ViewMatRefractDescriptorSky;
		vk::UniqueDescriptorSet	ViewMatReflectionDescriptor;
		vk::UniqueDescriptorSet	ViewMatReflectionDescriptorSky;
		vk::UniqueDescriptorSet	sandTexDescriptorSet[5];
		vk::UniqueDescriptorSet	cameraPosDescriptor;		
		vk::UniqueDescriptorSet	newViewCameraDescriptor;
		vk::UniqueDescriptorSet	cameraPosDescriptorBufferSky;
		vk::UniqueDescriptorSet fogDescriptor;
		vk::UniqueDescriptorSet fogDescriptorSky;
		vk::UniqueDescriptorSet lightDescriptor; 
		vk::UniqueDescriptorSet dudvTexDescriptor;		
		vk::UniqueDescriptorSet waterNormalTexDescriptor;
		vk::UniqueDescriptorSet ssboDescriptorDiffuse;
		vk::UniqueDescriptorSet ssboDescriptorDepth;
		vk::UniqueDescriptorSet ssboDescriptorDiffuse2;
		vk::UniqueDescriptorSet ssboDescriptorDepth2;
		vk::UniqueDescriptorSet clippingPlaneDescriptor[2]; 
		//vk::UniqueDescriptorSet lookupTableDescriptor;
		vk::UniqueDescriptorSetLayout lookupTableDescriptorLayout;
		
		Vector4 clippingPlane[2];
		Vector3 newCamPos;
		Vector3 newCamPos2;
		float far_plane;
		Matrix4 cubeProjMat;
		Matrix4 newView;
		std::vector<Matrix4> cubeViewMat;
		std::vector<Matrix4> cubeViewMatUpSideDown;

		struct Fog {
			Vector4 colour[3];
			float	colourMixMin = 20;
			float	colourMixMax = 40;

			float	gradient	= 1.5f;
			float	density		= 0.007f;

			Fog() {
				colour[0] = Vector4(0.3f, 0.6f, 0.8f, 1.0f);
				colour[1] = Vector4(0.2f, 0.4f, 0.5f, 1.0f);// //0.0f, 0.5f, 1.0f, 1.0f(0.2f, 0.75f, 1.0f, 1.0f)
				colour[2] = Vector4(0.0f, 0.2f, 0.3f, 1.0f);//0.0f,0.3f,0.4f(0.0f, 0.2f, 0.8f, 1.0f)

				gradient	= 1.5f;
				density		= 0.007f;

				colourMixMin = 20;
				colourMixMax = 40;
			}
			Fog(const float inGradient, float inDensity, const Vector4 inColour[3], const float mix[2]) {
				gradient = inGradient;
				density = inDensity;
				for (int i = 0; i < 3; ++i) {
					colour[i] = inColour[i];
				}
				colourMixMin = mix[0];
				colourMixMax = mix[1];
			}
		};

		struct Waves {
			Vector2 direction;
			float steepness;
			float waveLength;
			float speed;
			Waves() : direction(1.0, 1.0), steepness(0.5), waveLength(50.0), speed(1) {};
			Waves(Vector2 dir, float steep, float wlen, float spd) : direction(dir), steepness(steep), waveLength(wlen), speed(spd) {};
		};

		enum Meshes {
			cubeM,
			sphereM,
			gridM,
			quadM,

			totalM
		};

		UniqueVulkanMesh	meshes[totalM];

		enum Pipelines {
			skyboxB,		skyboxR,	//B - buffer, R - render
			objB,			objR,
			waveB,			waveR,
			cubeBufferP,
			//lookupTableP,
			totalP
		};

		VulkanPipeline		pipelines[totalP];

		enum DSet {		
			objDS,		objDS_B_Refract,	objDS_B_Reflect,
			skyboxDS,	skyboxDS_B_Refract, skyboxDS_B_Reflect,
			wavesDS,	
						cubeBuffer_Refract,	cubeBuffer_Reflect
		};

		std::vector<std::vector<vk::DescriptorSet>> DSv;
	};
}
