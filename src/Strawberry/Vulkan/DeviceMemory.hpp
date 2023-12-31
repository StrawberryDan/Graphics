#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Vulkan
#include <vulkan/vulkan.h>
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Vulkan
{
	class Device;


	class DeviceMemory
	{
		friend class Buffer;
		friend class Image;


	public:
		DeviceMemory() = default;
		DeviceMemory(const Device& device, uint32_t size, uint32_t flags, VkMemoryPropertyFlags properties = 0);
		DeviceMemory(const DeviceMemory& rhs) = delete;
		DeviceMemory& operator=(const DeviceMemory& rhs) = delete;
		DeviceMemory(DeviceMemory&& rhs);
		DeviceMemory& operator=(DeviceMemory&& rhs);
		~DeviceMemory();


		uint64_t GetSize() const;
		void SetData(const Strawberry::Core::IO::DynamicByteBuffer& bytes);


	private:
		VkDeviceMemory mDeviceMemory = nullptr;
		uint64_t mSize;
		VkDevice mDevice = nullptr;
		uint8_t* mMappedDataPtr = nullptr;
	};
}
