//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Surface.hpp"
#include "Strawberry/Window/Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"


//======================================================================================================================
//  Class Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics::Vulkan
{
	Surface::Surface(const Window::Window& window, const Device& device)
		: mDevice(device)
	{
		Core::AssertEQ(glfwCreateWindowSurface(device.GetInstance()->mInstance, window.mHandle, nullptr, &mSurface), GLFW_NO_ERROR);
	}


	Surface::Surface(Surface&& rhs) noexcept
		: mSurface(std::exchange(rhs.mSurface, nullptr))
		, mDevice(std::move(rhs.mDevice))
	{}


	Surface& Surface::operator=(Surface&& rhs)
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	Surface::~Surface()
	{
		if (mSurface)
		{
			vkDestroySurfaceKHR(mDevice->GetInstance()->mInstance, mSurface, nullptr);
		}
	}


	VkSurfaceCapabilitiesKHR Surface::GetCapabilities() const
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->GetPhysicalDevices()[0]->mPhysicalDevice, mSurface, &capabilities);
		return capabilities;
	}
}
