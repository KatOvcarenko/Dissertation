#include "Dissertation.h"
#include <VulkanRenderer.h>

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

Dissertation::Dissertation(Window& window) : VulkanTutorial(window) {
	VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME;
	VK_EXT_SCALAR_BLOCK_LAYOUT_SPEC_VERSION;
	
	VulkanInitialisation vkInit = DefaultInitialisation(); 
	vkInit.autoBeginDynamicRendering = false;
	renderer = new VulkanRenderer(window, vkInit);
	
	FrameState const& state = renderer->GetFrameState();
	vk::Device device = renderer->GetDevice();
	vk::DescriptorPool pool = renderer->GetDescriptorPool();

	InitTutorialObjects();

	meshes[Meshes::quadM] = GenerateQuad();
	meshes[Meshes::cubeM] = LoadMesh("Cube.msh");
	meshes[Meshes::sphereM] = LoadMesh("Sphere.msh");
	meshes[Meshes::gridM] = GenerateGrid1(2060);
	
	cubeTex2 = LoadCubemap(
		"Cubemap/side.JPG", "Cubemap/side.JPG",
		"Cubemap/UPnDown.JPG", "Cubemap/UPnDown.JPG",
		"Cubemap/side.JPG", "Cubemap/side.JPG",
		"Cubemap Texture!"
	);

	cubeTex = LoadCubemap(
		"Cubemap/Daylight Box_Right.png", "Cubemap/Daylight Box_Left.png",
		"Cubemap/Daylight Box_Top.png", "Cubemap/Daylight Box_Bottom.png",
		"Cubemap/Daylight Box_Front.png", "Cubemap/Daylight Box_Back.png",
		"Cubemap Texture!"
	);
	{sandTex[0] = LoadTexture("Ground/ground_0024_color_1k.jpg");
	sandTex[1] = LoadTexture("Ground/ground_0024_normal_opengl_1k.png");
	sandTex[2] = LoadTexture("Ground/ground_0024_roughness_1k.jpg");
	sandTex[3] = LoadTexture("Ground/ground_0024_height_1k.png");
	sandTex[4] = LoadTexture("Ground/ground_0024_ao_1k.jpg"); }
	dudvmapTex = LoadTexture("DUDV_map2.png");
	waterNormalTex = LoadTexture("normals_water.png");

	RENDERAREA = window.GetScreenSize().x;//cubeTex->GetDimensions().x;//

	camera.SetPosition({ 0, 10, 0 }).SetFarPlane(5000.0f);
	far_plane = 4000.0;
	cubeProjMat = Matrix::Perspective(1.0f, far_plane, 1, 90.f);

	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(0  , -90, camera.GetPosition()));
	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(0  , 90, camera.GetPosition()));
	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(90 , 0, camera.GetPosition()));
	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(-90, 0, camera.GetPosition()));
	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(0  , 0, camera.GetPosition()));
	cubeViewMat.push_back(cubeProjMat * Matrix::lookAt(0  , 180, camera.GetPosition()));

	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(180, -90, camera.GetPosition()));
	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(180, 90, camera.GetPosition()) );
	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(-90, 180, camera.GetPosition()));
	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(90, 180, camera.GetPosition()) );
	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(180, 180, camera.GetPosition()));
	cubeViewMatUpSideDown.push_back(cubeProjMat * Matrix::lookAt(180, 0, camera.GetPosition())  );

	{colours.push_back(Vector3(0, 0, 0));
	colours.push_back(Vector3(1, 0, 0));
	colours.push_back(Vector3(1, 0.5, 0));
	colours.push_back(Vector3(1, 1, 0));
	colours.push_back(Vector3(0, 1, 0));
	colours.push_back(Vector3(0, 1, 1));
	colours.push_back(Vector3(0, 0, 1));
	colours.push_back(Vector3(1, 0, 1));
	colours.push_back(Vector3(1, 1, 1)); }

	CreteUniforms();
	ShaderLoader();
	CreateSSBOBuffers(window.GetScreenSize().x, window.GetScreenSize().y);
	PipelinesBuilder();

	cubemapDescriptor = CreateDescriptorSet(device, pool, objectShader->GetLayout(1));
	WriteImageDescriptor(device, *cubemapDescriptor, 0, cubeTex->GetDefaultView(), *defaultSampler);

	cubemapDescriptor2 = CreateDescriptorSet(device, pool, skyboxShaderB->GetLayout(4));
	WriteImageDescriptor(device, *cubemapDescriptor2, 0, cubeTex2->GetDefaultView(), *defaultSampler);

	cameraPosDescriptor = CreateDescriptorSet(device, pool, objectShader->GetLayout(2));

	WriteBufferDescriptor(device, *cameraDescriptor, 0, vk::DescriptorType::eUniformBuffer, cameraBuffer);
	WriteBufferDescriptor(device, *cameraPosDescriptor, 0, vk::DescriptorType::eUniformBuffer, camPosUniform);

	for (int i = 0; i < 3; i++) {
		waveDescriptor[i] = CreateDescriptorSet(device, pool, waveShader->GetLayout(i + 5));
		WriteBufferDescriptor(device, *waveDescriptor[i], 0, vk::DescriptorType::eUniformBuffer, waveUniform[i]);
	}

	fogDescriptor = CreateDescriptorSet(device, pool, objectShader->GetLayout(3));
	WriteBufferDescriptor(device, *fogDescriptor, 0, vk::DescriptorType::eUniformBuffer, fogUniform);

	lightDescriptor = CreateDescriptorSet(device, pool, objectShader->GetLayout(4));
	WriteBufferDescriptor(device, *lightDescriptor, 0, vk::DescriptorType::eUniformBuffer, lightUniform);

	dudvTexDescriptor = CreateDescriptorSet(device, pool, waveShader->GetLayout(3));
	WriteImageDescriptor(device, *dudvTexDescriptor, 0, dudvmapTex->GetDefaultView(), *defaultSampler);

	waterNormalTexDescriptor = CreateDescriptorSet(device, pool, waveShader->GetLayout(9));
	WriteImageDescriptor(device, *waterNormalTexDescriptor, 0, waterNormalTex->GetDefaultView(), *defaultSampler);

	timeDescriptor = CreateDescriptorSet(device, pool, waveShader->GetLayout(8));
	WriteBufferDescriptor(device, *timeDescriptor, 0, vk::DescriptorType::eUniformBuffer, timeUniform);

	ViewMatRefractDescriptor = CreateDescriptorSet(device, pool, ssboThreeDBufferShader->GetLayout(1));
	WriteBufferDescriptor(device, *ViewMatRefractDescriptor, 0, vk::DescriptorType::eUniformBuffer, ViewMatUniformRefract);

	ViewMatRefractDescriptorSky = CreateDescriptorSet(device, pool, skyboxShaderB->GetLayout(5));
	WriteBufferDescriptor(device, *ViewMatRefractDescriptorSky, 0, vk::DescriptorType::eUniformBuffer, ViewMatUniformRefract);

	ViewMatReflectionDescriptor = CreateDescriptorSet(device, pool, ssboThreeDBufferShader->GetLayout(1));
	WriteBufferDescriptor(device, *ViewMatReflectionDescriptor, 0, vk::DescriptorType::eUniformBuffer, ViewMatUniformReflect);

	ViewMatReflectionDescriptorSky = CreateDescriptorSet(device, pool, skyboxShaderB->GetLayout(5));
	WriteBufferDescriptor(device, *ViewMatReflectionDescriptorSky, 0, vk::DescriptorType::eUniformBuffer, ViewMatUniformReflect);

	farPlaneDescriptor = CreateDescriptorSet(device, pool, ssboThreeDBufferShader->GetLayout(3));
	WriteBufferDescriptor(device, *farPlaneDescriptor, 0, vk::DescriptorType::eUniformBuffer, farPlaneUniform);

	newViewCameraDescriptor = CreateDescriptorSet(device, pool, objectShaderB->GetLayout(5));
	WriteBufferDescriptor(device, *newViewCameraDescriptor, 0, vk::DescriptorType::eUniformBuffer, newViewcameraBufferUniform);

	clippingPlaneDescriptor[0] = CreateDescriptorSet(device, pool, objectShaderB->GetLayout(6));
	clippingPlaneDescriptor[1] = CreateDescriptorSet(device, pool, objectShaderB->GetLayout(6));
	WriteBufferDescriptor(device, *clippingPlaneDescriptor[0], 0, vk::DescriptorType::eUniformBuffer, clippingPlaneUniform[0]);
	WriteBufferDescriptor(device, *clippingPlaneDescriptor[1], 0, vk::DescriptorType::eUniformBuffer, clippingPlaneUniform[1]);

	/*for (int i = 0; i < 5; ++i) {
		sandTexDescriptorSet[i] = CreateDescriptorSet(device, pool, shader->GetLayout(i));
		WriteImageDescriptor(device, *sandTexDescriptorSet[i], 0, sandTex[i]->GetDefaultView(), *defaultSampler);
	}*/

	lookup_table = readCSV("C:/Users/c2065496/Documents/VulkanTutorials/lookupTable.csv");
	UpdateDescriptors();
	CreteDescriptorSets();
}

void Dissertation::PipelinesBuilder() {
	FrameState const& state = renderer->GetFrameState();
	vk::Device device = renderer->GetDevice();

	pipelines[Pipelines::waveR] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::gridM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::gridM]->GetVulkanTopology())
		.WithShader(waveShader)
		.WithColourAttachment(state.colourFormat, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha)
		.WithDepthAttachment(state.depthFormat, vk::CompareOp::eLessOrEqual, true, true)
		.Build("Wave Pipeline Renderer");

	pipelines[Pipelines::skyboxR] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::quadM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::quadM]->GetVulkanTopology())
		.WithShader(skyboxShader)
		.WithColourAttachment(state.colourFormat)
		.WithDepthAttachment(state.depthFormat)
		.Build("Skybox Pipeline Renderer");

	pipelines[Pipelines::skyboxB] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::quadM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::quadM]->GetVulkanTopology())
		.WithShader(skyboxShaderB)
		.WithColourAttachment(ssboTexDiffuse->GetFormat())
		.WithDepthAttachment(ssboTexDepth->GetFormat())
		.Build("Skybox Pipeline Buffer");

	pipelines[Pipelines::objB] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::cubeM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::cubeM]->GetVulkanTopology())
		.WithShader(objectShaderB)
		.WithColourAttachment(ssboTexDiffuse->GetFormat())
		.WithDepthAttachment(ssboTexDepth->GetFormat(), vk::CompareOp::eLessOrEqual, true, true)
		.Build("object Pipeline Buffer");

	pipelines[Pipelines::objR] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::cubeM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::cubeM]->GetVulkanTopology())
		.WithShader(objectShader)
		.WithColourAttachment(state.colourFormat)
		.WithDepthAttachment(state.depthFormat, vk::CompareOp::eLessOrEqual, true, true)
		.Build("object Pipeline Render"); 

	pipelines[Pipelines::waterVolumeR]= PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::cubeM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::cubeM]->GetVulkanTopology())
		.WithShader(waterVolumeShader)
		.WithColourAttachment(state.colourFormat)
		.WithDepthAttachment(state.depthFormat, vk::CompareOp::eLessOrEqual, true, true)
		.Build("Water Volume Pipeline Render");

	/*ssboBufferPipeline = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::cubeM]->GetVertexInputState())
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(ssboBufferShader)
		.WithColourAttachment(bufferTextures[0]->GetFormat())
		.WithDepthAttachment(bufferTextures[1]->GetFormat(), vk::CompareOp::eLessOrEqual, true, true)
		.Build("ssbo Pipeline");*/

	pipelines[Pipelines::cubeBufferP] = PipelineBuilder(device)
		.WithVertexInputState(meshes[Meshes::quadM]->GetVertexInputState())
		.WithTopology(meshes[Meshes::quadM]->GetVulkanTopology())
		.WithShader(ssboThreeDBufferShader)
		.WithColourAttachment(state.colourFormat)
		.WithDepthAttachment(ssboTexDepth->GetFormat(), vk::CompareOp::eLessOrEqual, true, true)
		.Build("ssbo cube Pipeline");
}

void Dissertation::CreteDescriptorSets() {
	std::vector<vk::DescriptorSet> objD = {
	*cameraDescriptor,		//Set 0
	*cubemapDescriptor,		//Set 1
	*cameraPosDescriptor,	//Set 2
	*fogDescriptor,			//Set 3
	*lightDescriptor
	};

	std::vector<vk::DescriptorSet> objD_B_Refract = {
	*cameraDescriptor,		//Set 0
	*ViewMatRefractDescriptor,		//Set 1
	*cameraPosDescriptor,	//Set 2
	*fogDescriptor,			//Set 3
	*lightDescriptor,
	*newViewCameraDescriptor,
	*clippingPlaneDescriptor[0]
	};

	std::vector<vk::DescriptorSet> objD_B_Reflect = {
	*cameraDescriptor,		//Set 0
	*ViewMatReflectionDescriptor,		//Set 1
	*cameraPosDescriptor,	//Set 2
	*fogDescriptor,			//Set 3
	*lightDescriptor,
	*newViewCameraDescriptor,
	*clippingPlaneDescriptor[1]
	};

	std::vector<vk::DescriptorSet> skyboxD = {
	*cameraDescriptor, //Set 0
	*cubemapDescriptor, //Set 1
	*cameraPosDescriptor,
	*fogDescriptor,
	*cubemapDescriptor2
	};

	std::vector<vk::DescriptorSet> skyboxD_B_Refract = {
	*cameraDescriptor, //Set 0
	*cubemapDescriptor, //Set 1
	*cameraPosDescriptor,//2
	*fogDescriptor,//3
	*cubemapDescriptor2,//4
	*ViewMatRefractDescriptorSky
	};

	std::vector<vk::DescriptorSet> skyboxD_B_Reflect = {
	*cameraDescriptor, //Set 0
	*cubemapDescriptor, //Set 1
	*cameraPosDescriptor,//2
	*fogDescriptor,//3
	*cubemapDescriptor2,//4
	*ViewMatReflectionDescriptorSky
	};

	std::vector<vk::DescriptorSet> wavesD = {
	*cameraDescriptor,		//Set 0
	*cubemapDescriptor,		//Set 1
	*cameraPosDescriptor,	//Set 2
	*dudvTexDescriptor,		//Set 3	
	*lightDescriptor,
	*waveDescriptor[0],
	*waveDescriptor[1],
	*waveDescriptor[2],
	*timeDescriptor,
	*waterNormalTexDescriptor,
	*ssboDescriptorDiffuse,
	*ssboDescriptorDepth,
	*ssboDescriptorDiffuse2,
	*ssboDescriptorDepth2
	};

	std::vector<vk::DescriptorSet> watervolD = {
	*cameraDescriptor,		//Set 0
	*cubemapDescriptor,		//Set 1
	*cameraPosDescriptor,	//Set 2
	*fogDescriptor
	};
	
	std::vector<vk::DescriptorSet> cubeBuf_Refract = {
	*cameraDescriptor,	
	*ViewMatRefractDescriptor,
	*cameraPosDescriptor,
	*farPlaneDescriptor	
	};

	std::vector<vk::DescriptorSet> cubeBuf_Reflect = {
	*cameraDescriptor,
	*ViewMatReflectionDescriptor,
	*cameraPosDescriptor,
	*farPlaneDescriptor
	};

	DSv.push_back(objD);
	DSv.push_back(objD_B_Refract);
	DSv.push_back(objD_B_Reflect);
	DSv.push_back(skyboxD);
	DSv.push_back(skyboxD_B_Refract);
	DSv.push_back(skyboxD_B_Reflect);
	DSv.push_back(wavesD);
	DSv.push_back(watervolD);
	DSv.push_back(cubeBuf_Refract);
	DSv.push_back(cubeBuf_Reflect);
}

void Dissertation::CreateSSBOBuffers(uint32_t width, uint32_t height) {
	//TextureBuilder bufferTex(renderer->GetDevice(), renderer->GetMemoryAllocator());
	//{bufferTex.UsingPool(renderer->GetCommandPool(CommandBuffer::Graphics))
	//	.UsingQueue(renderer->GetQueue(CommandBuffer::Graphics))
	//	.WithDimension(width, height, 1)
	//	.WithMips(false)
	//	.WithUsages(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled/* | vk::ImageUsageFlagBits::eStorage*/)
	//	.WithPipeFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
	//	.WithLayout(vk::ImageLayout::eColorAttachmentOptimal)
	//	.WithFormat(vk::Format::eB8G8R8A8Unorm);
	//}
	//bufferTextures[0] = bufferTex.Build("buffer diffuse texture");
	//bufferTextures[1] = bufferTex
	//	.WithUsages(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
	//	.WithLayout(vk::ImageLayout::eDepthAttachmentOptimal)
	//	.WithFormat(vk::Format::eD32Sfloat)
	//	.WithAspects(vk::ImageAspectFlagBits::eDepth)
	//	.Build("buffer Depth texture");
	//vk::ImageViewType viewType = vk::ImageViewType::e2DArray;
	TextureBuilder bufferCubeTex(renderer->GetDevice(), renderer->GetMemoryAllocator());

	{bufferCubeTex.UsingPool(renderer->GetCommandPool(CommandBuffer::Graphics))
		.UsingQueue(renderer->GetQueue(CommandBuffer::Graphics))
		.WithDimension(RENDERAREA, RENDERAREA, 1)
		.WithMips(false)
		.WithUsages(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled/* | vk::ImageUsageFlagBits::eStorage*/)
		.WithPipeFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
		.WithLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.WithFormat(vk::Format::eB8G8R8A8Unorm)
		.WithAspects(vk::ImageAspectFlagBits::eColor);
	}

	ssboTexDiffuse = bufferCubeTex.BuildCubemap("buffer diffuse texture cube");
	cubeFaceView[0] = GenImageView(&*ssboTexDiffuse, vk::ImageAspectFlagBits::eColor);

	camera.GetPosition();//.SetPosition({ 0, 0, 0 }).SetFarPlane(5000.0f);

	ssboTexDepth = bufferCubeTex
		.WithUsages(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
		.WithLayout(vk::ImageLayout::eDepthAttachmentOptimal)
		.WithFormat(vk::Format::eD16Unorm)
		.WithAspects(vk::ImageAspectFlagBits::eDepth)
		.BuildCubemap("buffer Depth texture cube");
	cubeFaceView[1] = GenImageView(&*ssboTexDepth, vk::ImageAspectFlagBits::eDepth); 
	
	TextureBuilder bufferCubeTex2(renderer->GetDevice(), renderer->GetMemoryAllocator());

	{bufferCubeTex2.UsingPool(renderer->GetCommandPool(CommandBuffer::Graphics))
		.UsingQueue(renderer->GetQueue(CommandBuffer::Graphics))
		.WithDimension(RENDERAREA, RENDERAREA, 1)
		.WithMips(false)
		.WithUsages(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled/* | vk::ImageUsageFlagBits::eStorage*/)
		.WithPipeFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
		.WithLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.WithFormat(vk::Format::eB8G8R8A8Unorm)
		.WithAspects(vk::ImageAspectFlagBits::eColor);
	}

	ssboTexDiffuse2 = bufferCubeTex2.BuildCubemap("buffer diffuse texture cube2");
	cubeFaceView[2] = GenImageView(&*ssboTexDiffuse2, vk::ImageAspectFlagBits::eColor);

	ssboTexDepth2 = bufferCubeTex2
		.WithUsages(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
		.WithLayout(vk::ImageLayout::eDepthAttachmentOptimal)
		.WithFormat(vk::Format::eD16Unorm)
		.WithAspects(vk::ImageAspectFlagBits::eDepth)
		.BuildCubemap("buffer Depth texture cube2");
	cubeFaceView[3] = GenImageView(&*ssboTexDepth2, vk::ImageAspectFlagBits::eDepth);

	UpdateDescriptors();
}

vk::UniqueImageView Dissertation::GenImageView(VulkanTexture* tex, vk::ImageAspectFlags aspects) {
	vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
		.setViewType(vk::ImageViewType::e2DArray)
		.setFormat(tex->GetFormat())
		.setSubresourceRange(vk::ImageSubresourceRange(aspects, 0, 1, 0, 6))
		.setImage(tex->GetImage());

	return renderer->GetDevice().createImageViewUnique(viewInfo);
}

void Dissertation::ShaderLoader() {
	skyboxShader = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("skybox.vert.spv")
		.WithFragmentBinary("skybox.frag.spv")
		.Build("skybox Shader");

	skyboxShaderB = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("skyboxB.vert.spv")
		.WithGeometryBinary("skyboxB.geom.spv")
		.WithFragmentBinary("skyboxB.frag.spv")
		.Build("skybox Shader");

	objectShader = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("underwaterObject.vert.spv")
		.WithFragmentBinary("underwaterObject.frag.spv")
		.Build("obj Shader");

	objectShaderB = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("underwaterObjectB.vert.spv")
		.WithGeometryBinary("underwaterObjectB.geom.spv")
		.WithFragmentBinary("underwaterObjectB.frag.spv")
		.Build("obj Shader buffer");

	waterVolumeShader = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("WaterVolume.vert.spv")
		.WithFragmentBinary("WaterVolume.frag.spv")
		.Build("WaterVolume Shader");

	waveShader = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("WaterGerstnerWaves.vert.spv")
		.WithFragmentBinary("WaterGerstnerWaves.frag.spv")
		.Build("Wave Shader");

	//groundShader = ShaderBuilder(device)
	//	.WithVertexBinary(".vert.spv")
	//	.WithFragmentBinary(".frag.spv")
	//	.Build("Ground Shader");

	//ssboBufferShader = ShaderBuilder(renderer->GetDevice())
	//	.WithVertexBinary("BufferObj.vert.spv")
	//	.WithFragmentBinary("BufferObj.frag.spv")
	//	.Build("buffer Shader");

	ssboThreeDBufferShader = ShaderBuilder(renderer->GetDevice())
		.WithVertexBinary("CubeBuffer.vert.spv")
		.WithGeometryBinary("CubeBuffer.geom.spv")
		.WithFragmentBinary("CubeBuffer.frag.spv")
		.Build("cube buffer Shader");
}

void Dissertation::CreteUniforms() {
	camPosUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Vector3), "Camera Position");

	fogUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Fog), "Fog uniforms");

	lightUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Light), "Light uniforms");

	Waves waveInfos[3] = { {Vector2(1.0, 1.0), 0.25, 100, 5.0}, {Vector2(1.0, 0.6), 0.15, 51.0, 1.0},{Vector2(1.0, 1.3), 0.05, 30.0, 0.5} };

	for (int i = 0; i < 3; i++) {
		waveUniform[i] = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
			.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
			.WithHostVisibility()
			.Build(sizeof(Waves), "Wave uniforms");

		waveUniform[i].CopyData((void*)&waveInfos[i], sizeof(waveInfos[i]));
	}

	timeUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Fog), "Time uniform");

	farPlaneUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(float), "far plane uniform");

	ViewMatUniformRefract = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Matrix4) * 6, "Proj matrix uniform");

	ViewMatUniformReflect = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Matrix4) * 6, "view matrix reflection uniform");

	newViewcameraBufferUniform = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Matrix4), "new view matrix uniform");

	clippingPlaneUniform[0] = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Vector4), "clipping plane uniform");

	clippingPlaneUniform[1] = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(sizeof(Vector4), "-clipping plane uniform");

	Fog fogInfo;
	Light lightInfo(Vector3(0, 150, -50), 100, Vector4(1.0, 1.0, 1.0, 1.0));
	clippingPlane[0] = Vector4(0.0, 1.0, 0.0, 0.0);
	clippingPlane[1] = Vector4(0.0, -1.0, 0.0, 0.0);

	clippingPlaneUniform[0].CopyData((void*)&clippingPlane[0], sizeof(Vector4));
	clippingPlaneUniform[1].CopyData((void*)&clippingPlane[1], sizeof(Vector4));
	fogUniform.CopyData((void*)&fogInfo, sizeof(fogInfo));
	lightUniform.CopyData((void*)&lightInfo, sizeof(lightInfo));
	farPlaneUniform.CopyData((void*)&far_plane, sizeof(far_plane));
}

std::vector<std::vector<std::string>> Dissertation::readCSV(const std::string& filePath) {
	std::vector<std::vector<std::string>> data;
	std::ifstream file(filePath);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open the file." << std::endl;
		return data;
	}

	char bom[3];
	file.read(bom, 3);
	if (bom[0] != '\xEF' || bom[1] != '\xBB' || bom[2] != '\xBF') {
		file.seekg(0);
	}

	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> row;
		std::stringstream ss(line);
		std::string value;

		while (std::getline(ss, value, ',')) {
			row.push_back(value);
		}

		data.push_back(row);
	}

	file.close();
	//for (const auto& row : data) {
	//	for (const auto& value : row) {
	//		std::cout << value << " "; 
	//	}
	//	std::cout << std::endl;
	//}
	
	return data;
}

void Dissertation::RenderFrame(float dt) {
	timeUniform.CopyData((void*)&runTime, sizeof(runTime));

	FillBufferCube();
	FillBufferCubeUpSideDown();

	TransitionColourToSampler(renderer->GetFrameState().cmdBuffer, *ssboTexDiffuse);
	TransitionDepthToSampler(renderer->GetFrameState().cmdBuffer, *ssboTexDepth);
	TransitionColourToSampler(renderer->GetFrameState().cmdBuffer, *ssboTexDiffuse2);
	TransitionDepthToSampler(renderer->GetFrameState().cmdBuffer, *ssboTexDepth2);

	renderer->BeginDefaultRendering(renderer->GetFrameState().cmdBuffer);

	DrawSkyBox(Pipelines::skyboxR, DSet::skyboxDS);
	ColourCheck(Pipelines::objR, DSet::objDS);
	DrawWaves(Pipelines::waveR);

	renderer->GetFrameState().cmdBuffer.endRendering();

	TransitionSamplerToColour(renderer->GetFrameState().cmdBuffer, *ssboTexDiffuse);
	TransitionSamplerToDepth(renderer->GetFrameState().cmdBuffer, *ssboTexDepth);
	TransitionSamplerToColour(renderer->GetFrameState().cmdBuffer, *ssboTexDiffuse2);
	TransitionSamplerToDepth(renderer->GetFrameState().cmdBuffer, *ssboTexDepth2);
}

void Dissertation::FillBufferCube() {
	FrameState const& frameState = renderer->GetFrameState();
	vk::Viewport newViewport = vk::Viewport(0.0f, (float)RENDERAREA, (float)RENDERAREA, -(float)RENDERAREA, 0.0f, 1.0f);
	vk::Rect2D	 newScissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(RENDERAREA, RENDERAREA));

	cubeViewMat[0] = cubeProjMat * Matrix::lookAt(0, -90, camera.GetPosition());
	cubeViewMat[1] = cubeProjMat * Matrix::lookAt(0, 90, camera.GetPosition());
	cubeViewMat[2] = cubeProjMat * Matrix::lookAt(90, 0, camera.GetPosition());
	cubeViewMat[3] = cubeProjMat * Matrix::lookAt(-90, 0, camera.GetPosition());
	cubeViewMat[4] = cubeProjMat * Matrix::lookAt(0, 0, camera.GetPosition());
	cubeViewMat[5] = cubeProjMat * Matrix::lookAt(0, 180, camera.GetPosition());

	ViewMatUniformRefract.CopyData((void*)cubeViewMat.data(), sizeof(Matrix4) * 6);
	newCamPos = camera.GetPosition();
	camPosUniform.CopyData(&newCamPos, sizeof(Vector3));

	newView = Matrix::lookAt(camera.GetPitch(), camera.GetYaw(), camera.GetPosition());
	newViewcameraBufferUniform.CopyData(&newView, sizeof(Matrix4));

	frameState.cmdBuffer.beginRendering(
		DynamicRenderBuilder()
		.WithColourAttachment(*cubeFaceView[0], vk::ImageLayout::eColorAttachmentOptimal, true, vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f))//
		.WithDepthAttachment(*cubeFaceView[1], vk::ImageLayout::eDepthAttachmentOptimal, true, { {1.0f} }, false)
		.WithRenderArea(newScissor)
		.WithLayerCount(6)
		.Build()
	);
	frameState.cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[Pipelines::cubeBufferP]);

	frameState.cmdBuffer.setViewport(0, 1, &newViewport);
	frameState.cmdBuffer.setScissor(0, 1, &newScissor);
	frameState.cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[Pipelines::cubeBufferP].layout, 0, DSv[DSet::cubeBuffer_Refract].size(), DSv[DSet::cubeBuffer_Refract].data(), 0, nullptr);

	DrawSkyBox(Pipelines::skyboxB, DSet::skyboxDS_B_Refract);
	ColourCheck(Pipelines::objB, DSet::objDS_B_Refract);

	frameState.cmdBuffer.endRendering();
}

void Dissertation::FillBufferCubeUpSideDown() {
	FrameState const& frameState = renderer->GetFrameState();
	vk::Viewport newViewport = vk::Viewport(0.0f, (float)RENDERAREA, (float)RENDERAREA, -(float)RENDERAREA, 0.0f, 1.0f);
	vk::Rect2D	 newScissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(RENDERAREA, RENDERAREA));

	frameState.cmdBuffer.beginRendering(
		DynamicRenderBuilder()
		.WithColourAttachment(*cubeFaceView[2], vk::ImageLayout::eColorAttachmentOptimal, true, vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f))//
		.WithDepthAttachment(*cubeFaceView[3], vk::ImageLayout::eDepthAttachmentOptimal, true, { {1.0f} }, false)
		.WithRenderArea(newScissor)
		.WithLayerCount(6)
		.Build()
	);
	frameState.cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[Pipelines::cubeBufferP]);
	InvertCamera();
	newView = Matrix::lookAt(-camera.GetPitch(), camera.GetYaw(), newCamPos);
	newViewcameraBufferUniform.CopyData(&newView, sizeof(Matrix4));

	cubeViewMatUpSideDown[0] = cubeProjMat * Matrix::lookAt(0, -90, newCamPos);	//cubeProjMat * Matrix::lookAt(180, -90, newCamPos);
	cubeViewMatUpSideDown[1] = cubeProjMat * Matrix::lookAt(0, 90, newCamPos);	//cubeProjMat * Matrix::lookAt(180, 90, newCamPos);
	cubeViewMatUpSideDown[2] = cubeProjMat * Matrix::lookAt(90, 0, newCamPos);	//cubeProjMat * Matrix::lookAt(-90, 180, newCamPos);
	cubeViewMatUpSideDown[3] = cubeProjMat * Matrix::lookAt(-90, 0, newCamPos);	//cubeProjMat * Matrix::lookAt(90, 180, newCamPos);
	cubeViewMatUpSideDown[4] = cubeProjMat * Matrix::lookAt(0, 0, newCamPos);	//cubeProjMat * Matrix::lookAt(180, 180, newCamPos);
	cubeViewMatUpSideDown[5] = cubeProjMat * Matrix::lookAt(0, 180, newCamPos);	//cubeProjMat * Matrix::lookAt(180, 0, newCamPos);
	ViewMatUniformReflect.CopyData((void*)cubeViewMatUpSideDown.data(), sizeof(Matrix4) * 6);

	frameState.cmdBuffer.setViewport(0, 1, &newViewport);
	frameState.cmdBuffer.setScissor(0, 1, &newScissor);
	frameState.cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[Pipelines::cubeBufferP].layout, 0, DSv[DSet::cubeBuffer_Reflect].size(), DSv[DSet::cubeBuffer_Reflect].data(), 0, nullptr);

	DrawSkyBox(Pipelines::skyboxB, DSet::skyboxDS_B_Reflect); 
	ColourCheck(Pipelines::objB, DSet::objDS_B_Reflect);

	frameState.cmdBuffer.endRendering();
	InvertCamera();
}

void Dissertation::InvertCamera() {
	newCamPos = Vector3(camera.GetPosition().x, -camera.GetPosition().y, camera.GetPosition().z);
	camPosUniform.CopyData(&newCamPos, sizeof(Vector3));
}

void Dissertation::UpdateDescriptors() {

	ssboDescriptorDiffuse = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(10));
	WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptorDiffuse, 0, ssboTexDiffuse->GetDefaultView(), *defaultSampler);
	ssboDescriptorDepth = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(11));
	WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptorDepth, 0, ssboTexDepth->GetDefaultView(), *defaultSampler, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
	
	ssboDescriptorDiffuse2 = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(12));
	WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptorDiffuse2, 0, ssboTexDiffuse2->GetDefaultView(), *defaultSampler);
	ssboDescriptorDepth2 = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(13));
	WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptorDepth2, 0, ssboTexDepth2->GetDefaultView(), *defaultSampler, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
	
	cameraPosDescriptor = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), ssboThreeDBufferShader->GetLayout(2));
	WriteBufferDescriptor(renderer->GetDevice(), *cameraPosDescriptor, 0, vk::DescriptorType::eUniformBuffer, camPosUniform);
	//ssboDescriptor[0] = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(10));
	//WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptor[0], 0, bufferTextures[0]->GetDefaultView(), *defaultSampler);
	//ssboDescriptor[1] = CreateDescriptorSet(renderer->GetDevice(), renderer->GetDescriptorPool(), waveShader->GetLayout(11));
	//WriteImageDescriptor(renderer->GetDevice(), *ssboDescriptor[1], 0, bufferTextures[1]->GetDefaultView(), *defaultSampler, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
}

void Dissertation::DrawSkyBox(const int num, const int DS) {
	FrameState const& frameState = renderer->GetFrameState();
	vk::CommandBuffer cmdBuffer = frameState.cmdBuffer;

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[num]);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[num].layout, 0, DSv[DS].size(), DSv[DS].data(), 0, nullptr);
	meshes[Meshes::quadM]->Draw(cmdBuffer);
}

void Dissertation::ColourCheck(const int numOfP, const int DS) {

	Vector3 startpos = Vector3(-10, -110, -50);
	Vector3 pos = startpos;

	for (int i = 0; i < 3; i++) {
		pos = startpos;
		pos.z -= i * 70;
		for (int col = 0; col < colours.size(); col++) {
			DrawObj(pos, Vector3(2, 200, 2), Vector4(colours[col], 1), DS, numOfP, Meshes::cubeM);
			pos.x += 2;
		}
	}
	
	pos = Vector3(50, -100, -30);

	for (int col = 0; col < colours.size(); col++) {
		DrawObj(pos, Vector3(2, 2, 400), Vector4(colours[col], 1), DS, numOfP, Meshes::cubeM, -15);
		pos.x += 2;
	}

	float radius = 20.0f; 
	int numCubes = colours.size();

	for (int j = 0; j < 3; j++) {
		pos = Vector3(-12, -100 + 40 * j, -70);
		pos.z -= j * 70; 

		for (int i = 0; i < numCubes; i++) {
			float angle = i * (360.0f / numCubes);
			float radian = Maths::DegreesToRadians(angle);

			pos.x += radius * cos(radian);
			pos.z += radius * sin(radian);
			//pos.y += 2 * cos(runTime) - sin(runTime);
			pos.y = 40 * cos(runTime) - sin(runTime);
			DrawObj(pos, Vector3(3, 3, 3), Vector4(colours[i % colours.size()], 1), DS, numOfP, Meshes::sphereM);
		}
	}
																					
}

void Dissertation::DrawObj(const Vector3& translation, const Vector3& scale, const Vector4& colour, const int DS, const int& pipeline, const int& meshName,
	const float& angle, const Vector3& axis) {
	FrameState const& frameState = renderer->GetFrameState();
	vk::CommandBuffer cmdBuffer = frameState.cmdBuffer;
	vk::Device device = renderer->GetDevice();

	Matrix4 objectModelMatrix = Matrix::Translation(translation) * Matrix::Rotation(angle, axis) * Matrix::Scale(scale);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[pipeline]);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Matrix4), (void*)&objectModelMatrix);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eFragment, sizeof(float)*16, sizeof(colour), (void*)&colour);

	//vk::DescriptorSet objectSets[] = {
	//	*cameraDescriptor,		//Set 0
	//	*cubemapDescriptor,		//Set 1
	//	*cameraPosDescriptor,	//Set 2
	//	*fogDescriptor,			//Set 3
	//	*lightDescriptor
	//};
	//cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[pipeline].layout, 0, sizeof(objectSets) / sizeof(objectSets[0]), objectSets, 0, nullptr);
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[pipeline].layout, 0, DSv[DS].size(), DSv[DS].data(), 0, nullptr);
	meshes[meshName]->Draw(cmdBuffer);
}

void Dissertation::DrawWaterVolume(const Vector3& translation, const Vector3& scale, const Vector4& colour, const float angle, const Vector3& axis, const int pipeline) {

	FrameState const& frameState = renderer->GetFrameState();
	vk::CommandBuffer cmdBuffer = frameState.cmdBuffer;
	vk::Device device = renderer->GetDevice();

	Matrix4 objectModelMatrix = Matrix::Translation(translation) * Matrix::Rotation(angle, axis) * Matrix::Scale(scale);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[pipeline]);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Matrix4), (void*)&objectModelMatrix);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eFragment, sizeof(Matrix4), sizeof(colour), (void*)&colour);

	vk::DescriptorSet objectSets[] = {
		*cameraDescriptor,		//Set 0
		*cubemapDescriptor,		//Set 1
		*cameraPosDescriptor,	//Set 2
		*fogDescriptor
	};

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[pipeline].layout, 0, sizeof(objectSets) / sizeof(objectSets[0]), objectSets, 0, nullptr);

	meshes[Meshes::cubeM]->Draw(cmdBuffer);
}

void Dissertation::DrawWaves(const int pipeline) {
	FrameState const& frameState = renderer->GetFrameState();
	vk::CommandBuffer cmdBuffer = frameState.cmdBuffer;
	vk::Device device = renderer->GetDevice();
	Matrix4 objectModelMatrix = Matrix::Translation(Vector3(0, 0, 0));
	Vector4 colour = Vector4(0.6, 0.9, 0.8, 1.0);//(0.2f, 0.75f, 1.0f, 1.0f);(0.1, 0.19, 0.22, 1.0);//

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines[pipeline]);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Matrix4), (void*)&objectModelMatrix);
	cmdBuffer.pushConstants(*pipelines[pipeline].layout, vk::ShaderStageFlagBits::eFragment, sizeof(Matrix4), sizeof(colour), (void*)&colour);

	//vk::DescriptorSet objectSets[] = {
	//	*cameraDescriptor,		//Set 0
	//	*cubemapDescriptor,		//Set 1
	//	*cameraPosDescriptor,	//Set 2
	//	*dudvTexDescriptor,		//Set 3	
	//	*lightDescriptor,
	//	*waveDescriptor[0],
	//	*waveDescriptor[1],
	//	*waveDescriptor[2],
	//	*timeDescriptor,
	//	*waterNormalTexDescriptor,
	//	//*ssboDescriptor[0],
	//	//*ssboDescriptor[1],
	//	*ssboDescriptorDiffuse,
	//	*ssboDescriptorDepth
	//};
	//cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[pipeline].layout, 0, sizeof(objectSets) / sizeof(objectSets[0]), objectSets, 0, nullptr);
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelines[pipeline].layout, 0, DSv[DSet::wavesDS].size(), DSv[DSet::wavesDS].data(), 0, nullptr);

	meshes[Meshes::gridM]->Draw(cmdBuffer);
}