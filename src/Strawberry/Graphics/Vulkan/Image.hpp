#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "DeviceMemory.hpp"
// Strawberry Core
#include "Strawberry/Core/Math/Vector.hpp"
// Vulkan
#include <vulkan/vulkan.h>



//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics::Vulkan
{
	class Device;


	class Image
	{
		friend class ImageView;
		friend class CommandBuffer;


	public:
		Image(const Device& device,
			  uint32_t extent,
			  VkFormat format,
			  VkImageUsageFlags usage,
			  uint32_t mipLevels = 1,
			  uint32_t arrayLayers = 1,
			  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

		Image(const Device& device,
			  Core::Math::Vec2i extent,
			  VkFormat format,
			  VkImageUsageFlags usage,
			  uint32_t mipLevels = 1,
			  uint32_t arrayLayers = 1,
			  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

		Image(const Device& device,
			  Core::Math::Vec3i extent,
			  VkFormat format,
			  VkImageUsageFlags usage,
			  uint32_t mipLevels = 1,
			  uint32_t arrayLayers = 1,
			  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

		Image(const Image& rhs) = delete;
		Image& operator=(const Image& rhs) = delete;
		Image(Image&& rhs) noexcept ;
		Image& operator=(Image&& rhs) noexcept;
		~Image();


		template <std::movable T, typename... Args>
		T Create(Args... args) const { return T(*this, std::forward<Args>(args)...); }


		Core::Math::Vec3i GetSize() const;


	private:
		VkImage mImage;
		DeviceMemory mMemory;
		VkDevice mDevice;
		Core::Math::Vec3i mSize;
	};
}