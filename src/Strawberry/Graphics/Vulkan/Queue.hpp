#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Vulkan
#include <vulkan/vulkan.h>


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics::Vulkan
{
	class Device;
	class CommandBuffer;


	class Queue
	{
		friend class Swapchain;


	public:
		explicit Queue(const Device& device);
		Queue(const Queue& rhs) = delete;
		Queue& operator=(const Queue& rhs) = delete;
		Queue(Queue&& rhs) noexcept;
		Queue& operator=(Queue&& rhs);
		~Queue();


		void Submit(const CommandBuffer& commandBuffer);


	private:
		VkQueue mQueue;
		VkFence mSubmissionFence;
		VkDevice mDevice;
	};
}