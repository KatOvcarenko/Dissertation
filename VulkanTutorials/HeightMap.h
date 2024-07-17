#pragma once
#include "VulkanTutorial.h"
#include <vulkan/vulkan.hpp>
#include "VulkanMesh.h"

namespace NCL::Rendering::Vulkan {
	class HeightMap: public VulkanTutorial
	{
	public:
		HeightMap(int width, int height);
		HeightMap(Window& window); 

		Vector3 GetHeightmapSize() const { return heightmapSize; }

		int GetWidth() { return mapWidth; }
		int GetHeight() { return mapHeight; }

		void SetWidth(int num) { mapWidth = num; }
		void SetHeight(int num) { mapHeight = num; }
	protected:
		Vector3 heightmapSize;
		int mapWidth, mapHeight, mapChans;
	};
}

