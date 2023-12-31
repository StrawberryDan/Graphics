//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "Framebuffer.hpp"
#include "Fence.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Queue.hpp"
#include "DescriptorSet.hpp"
// Strawberry Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Math/Vector.hpp"
// Standard Library
#include <memory>
#include <vector>


//======================================================================================================================
//  Class Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Vulkan
{
	CommandBuffer::CommandBuffer(const CommandPool& commandPool)
		: mCommandBuffer {}
		, mCommandPool(commandPool)
		, mQueueFamilyIndex(mCommandPool->GetQueue()->GetFamilyIndex())
	{
		VkCommandBufferAllocateInfo allocateInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = mCommandPool->mCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		Core::Assert(vkAllocateCommandBuffers(mCommandPool->GetQueue()->GetDevice()->mDevice, &allocateInfo, &mCommandBuffer) == VK_SUCCESS);
	}


	CommandBuffer::CommandBuffer(CommandBuffer&& rhs) noexcept
		: mCommandBuffer(std::exchange(rhs.mCommandBuffer, nullptr))
		  , mCommandPool(std::move(rhs.mCommandPool))
		  , mQueueFamilyIndex(std::exchange(rhs.mQueueFamilyIndex, 0))
	{

	}


	CommandBuffer& CommandBuffer::operator=(CommandBuffer&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	CommandBuffer::~CommandBuffer()
	{
		if (mCommandBuffer)
		{
			vkFreeCommandBuffers(mCommandPool->GetQueue()->GetDevice()->mDevice, mCommandPool->mCommandPool, 1, &mCommandBuffer);
		}
	}


	Core::ReflexivePointer<CommandPool> CommandBuffer::GetCommandPool() const
	{
		return mCommandPool;
	}


	void CommandBuffer::Begin(bool oneTimeSubmit)
	{
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VkCommandBufferUsageFlags(0),
			.pInheritanceInfo = nullptr,
		};

		Core::AssertEQ(vkBeginCommandBuffer(mCommandBuffer, &beginInfo), VK_SUCCESS);
	}


	void CommandBuffer::End()
	{
		Core::AssertEQ(vkEndCommandBuffer(mCommandBuffer), VK_SUCCESS);
	}


	void CommandBuffer::Reset()
	{
		Core::AssertEQ(vkResetCommandBuffer(mCommandBuffer, 0), VK_SUCCESS);
	}


	void CommandBuffer::BindPipeline(const Pipeline& pipeline)
	{
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
	}


	void CommandBuffer::BindVertexBuffer(uint32_t binding, Buffer& buffer)
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(mCommandBuffer, binding, 1, &buffer.mBuffer, &offset);
	}


	void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		vkCmdDraw(mCommandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
	}


	void CommandBuffer::ImageMemoryBarrier(Image& image, VkImageAspectFlagBits aspect,
	                                       VkImageLayout targetLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
	{
		ImageMemoryBarrier(image.mImage, aspect, image.mLastRecordedLayout, targetLayout);
		image.mLastRecordedLayout = targetLayout;
	}


	void CommandBuffer::ImageMemoryBarrier(VkImage image, VkImageAspectFlagBits aspect, VkImageLayout oldLayout,
	                                       VkImageLayout targetLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
	{
		VkImageMemoryBarrier imageMemoryBarrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = srcAccessMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = targetLayout,
			.srcQueueFamilyIndex = mQueueFamilyIndex,
			.dstQueueFamilyIndex = mQueueFamilyIndex,
			.image = image,
			.subresourceRange{
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		vkCmdPipelineBarrier(mCommandBuffer,
							 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
							 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
							 0,
							 0, nullptr, 0, nullptr, 1,
							 &imageMemoryBarrier);
	}


	void CommandBuffer::CopyImageToSwapchain(Image& image, Swapchain& swapchain)
	{
		ImageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		ImageMemoryBarrier(swapchain.GetNextImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


		VkImageBlit region {
			.srcSubresource {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets{
				{.x = 0, .y = 0, .z = 0},
				{.x = static_cast<int32_t>(image.mSize[0]), .y = static_cast<int32_t>(image.mSize[1]), .z = 1},
			},
			.dstSubresource {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets{
				{.x = 0, .y = 0, .z = 0},
				{.x = swapchain.GetSize()[0], .y = swapchain.GetSize()[1], .z = 1},
			}
		};
		vkCmdBlitImage(mCommandBuffer, image.mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.GetNextImage(),
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
	}


	void CommandBuffer::CopyBufferToImage(const Buffer& buffer, Image& image)
	{
		// Clear and put into DST_OPTIMAL
		ImageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Setup copy
		VkImageSubresourceLayers subresource {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		VkBufferImageCopy region {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = subresource,
			.imageOffset{.x = 0, .y = 0, .z = 0},
			.imageExtent{.width = image.mSize[0], .height = image.mSize[1], .depth = 1}
		};
		vkCmdCopyBufferToImage(mCommandBuffer, buffer.mBuffer, image.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		image.mBytes = buffer.GetBytes();
	}


	void CommandBuffer::ClearColorImage(Image& image, Core::Math::Vec4f clearColor)
	{
		VkClearColorValue vulkanClearColor {
			.float32{clearColor[0], clearColor[1], clearColor[2], clearColor[3]}
		};

		VkImageSubresourceRange range {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1,
			0, 1
		};

		ImageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
		vkCmdClearColorImage(mCommandBuffer, image.mImage, VK_IMAGE_LAYOUT_GENERAL, &vulkanClearColor, 1, &range);
	}


	void CommandBuffer::BindDescriptorSet(const Pipeline& pipeline, uint32_t set, const DescriptorSet& descriptorSet)
	{
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipelineLayout, set, 1, &descriptorSet.mDescriptorSet, 0, nullptr);
	}


	void CommandBuffer::BeginRenderPass(const RenderPass& renderPass, Framebuffer& framebuffer)
	{
		for (int i = 0; i < framebuffer.GetColorAttachmentCount(); i++)
		{
			ImageMemoryBarrier(framebuffer.GetColorAttachment(i), VK_IMAGE_ASPECT_COLOR_BIT,
			                   VK_IMAGE_LAYOUT_GENERAL);
		}
		ImageMemoryBarrier(framebuffer.GetDepthAttachment(), VK_IMAGE_ASPECT_DEPTH_BIT,
		                   VK_IMAGE_LAYOUT_GENERAL);
		ImageMemoryBarrier(framebuffer.GetStencilAttachment(), VK_IMAGE_ASPECT_STENCIL_BIT,
		                   VK_IMAGE_LAYOUT_GENERAL);


		VkRenderPassBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = renderPass.mRenderPass,
			.framebuffer = framebuffer.mFramebuffer,
			.renderArea{.offset{0, 0}, .extent{static_cast<uint32_t>(framebuffer.mSize[0]),
											   static_cast<uint32_t>(framebuffer.mSize[1])}},
			.clearValueCount = static_cast<uint32_t>(renderPass.mClearColors.size()),
			.pClearValues = renderPass.mClearColors.data(),
		};
		vkCmdBeginRenderPass(mCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}


	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(mCommandBuffer);
	}


	void CommandBuffer::PushConstants(const Pipeline& pipeline, VkShaderStageFlags stage, const Core::IO::DynamicByteBuffer& bytes, uint32_t offset)
	{
		vkCmdPushConstants(mCommandBuffer, pipeline.mPipelineLayout, stage, offset, bytes.Size(), bytes.Data());
	}
}
