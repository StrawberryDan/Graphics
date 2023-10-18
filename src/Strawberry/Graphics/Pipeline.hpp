#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Vulkan
#include <vulkan/vulkan.h>


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics
{
	class Device;


	class Pipeline
	{
		friend class Builder;


	public:
		class Builder;


	public:
		Pipeline(const Pipeline& rhs) = delete;
		Pipeline& operator=(const Pipeline& rhs) = delete;
		Pipeline(Pipeline&& rhs) noexcept ;
		Pipeline& operator=(Pipeline&& rhs) noexcept ;
		~Pipeline();


	private:
		Pipeline() = default;


	private:
		VkPipeline mPipeline = nullptr;
		VkDevice mDevice = nullptr;
	};


	class Pipeline::Builder
	{
	public:
		explicit Builder(const Device& device);


		[[nodiscard]] Pipeline Build() const;

	private:
		const Device* mDevice;
	};
}
