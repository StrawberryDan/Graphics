
#include "Strawberry/Core/UTF.hpp"
#include "../src/Strawberry/Vulkan/Instance.hpp"
#include "../src/Strawberry/Vulkan/Device.hpp"
#include "../src/Strawberry/Vulkan/Pipeline.hpp"
#include "../src/Strawberry/Vulkan/Swapchain.hpp"
#include "../src/Strawberry/Vulkan/Queue.hpp"
#include "../src/Strawberry/Vulkan/CommandPool.hpp"
#include "../src/Strawberry/Vulkan/CommandBuffer.hpp"
#include "../src/Strawberry/Vulkan/Buffer.hpp"
#include "../src/Strawberry/Vulkan/Image.hpp"
#include "../src/Strawberry/Vulkan/Shader.hpp"
#include "../src/Strawberry/Vulkan/Framebuffer.hpp"
#include "Strawberry/Core/Math/Matrix.hpp"
#include "Strawberry/Core/Timing/Clock.hpp"
#include "../src/Strawberry/Vulkan/Sampler.hpp"
#include "../src/Strawberry/Vulkan/RenderPass.hpp"
#include "../src/Strawberry/Vulkan/Surface.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
#include "Strawberry/Window/Window.hpp"


using namespace Strawberry;
using namespace Vulkan;


void BasicRendering()
{
	uint8_t meshVertexShader[] =
	{
		#include "Mesh.vert.bin"
	};

	uint8_t solidColorFragShader[] =
	{
		#include "SolidColor.frag.bin"
	};

	uint8_t textureFragShader[] =
	{
		#include "Texture.frag.bin"
	};

	auto vertexInputDescription = []() -> VertexInputDescription
	{
		VertexInputDescription description;
		description.AddBinding(3 * sizeof(float))
			.WithAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
		return description;
	};


	Window::Window window("StrawberryGraphics Test", Core::Math::Vec2i(1920, 1080));
	Instance instance;
	const PhysicalDevice& gpu = instance.GetPhysicalDevices()[0];
	uint32_t queueFamily = gpu.SearchQueueFamilies(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)[0];


	Vulkan::Device device(gpu, {QueueCreateInfo{queueFamily, 1}});
	Vulkan::Surface surface = window.Create<Surface>(device);
	RenderPass renderPass = RenderPass::Builder(device)
		.WithColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
		.WithSubpass(SubpassDescription().WithColorAttachment(0))
		.Build();
	auto vertexShader = Shader::Compile(device, Core::IO::DynamicByteBuffer(meshVertexShader, sizeof(meshVertexShader))).Unwrap();
	auto fragmentShader = Shader::Compile(device, Core::IO::DynamicByteBuffer(textureFragShader, sizeof(textureFragShader))).Unwrap();
	Pipeline pipeline = renderPass.Create<Pipeline::Builder>()
		.WithShaderStage(VK_SHADER_STAGE_VERTEX_BIT, std::move(vertexShader))
		.WithShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, std::move(fragmentShader))
		.WithVertexInput(vertexInputDescription())
		.WithPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.WithViewport({0, 0}, {1920, 1080})
		.WithPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 16 * sizeof(float), 0)
		.WithPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 3 * sizeof(float), 16 * sizeof(float))
		.WithDescriptorSetLayout(
			DescriptorSetLayout()
				.WithBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT))
		.Build();
	auto queue = device.GetQueue(queueFamily, 0);
	Vulkan::Swapchain swapchain = queue->Create<Swapchain>(surface, Core::Math::Vec2i(1920, 1080));
	Vulkan::CommandPool commandPool = queue->Create<CommandPool>(true);
	Vulkan::CommandBuffer commandBuffer = commandPool.Create<CommandBuffer>();


	Buffer buffer(device, 6 * sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	Core::IO::DynamicByteBuffer vertices;
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(0.0f, 0.0f, 0.0f));
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(1.0f, 0.0f, 0.0f));
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(0.0f, 1.0f, 0.0f));
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(0.5f, 0.5f, 0.0f));
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(1.0f, 0.5f, 0.0f));
	vertices.Push<Core::Math::Vec3f>(Core::Math::Vec3f(0.5f, 1.0f, 0.0f));
	buffer.SetData(vertices);


	auto framebuffer = renderPass.Create<Framebuffer>(Core::Math::Vec2u(1920, 1080));


	auto [size, channels, bytes] = Core::IO::DynamicByteBuffer::FromImage("data/dio.png").Unwrap();
	Buffer textureBuffer(device, bytes.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	textureBuffer.SetData(bytes);
	Image texture(device, size, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	commandBuffer.Begin(true);
	commandBuffer.CopyBufferToImage(textureBuffer, texture);
	commandBuffer.ImageMemoryBarrier(texture, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
	commandBuffer.End();
	queue->Submit(commandBuffer);
	queue->WaitUntilIdle();
	ImageView textureView = texture.Create<ImageView::Builder>()
		.WithType(VK_IMAGE_VIEW_TYPE_2D)
		.WithFormat(VK_FORMAT_R8G8B8A8_SRGB)
		.Build();
	Sampler sampler(device, VK_FILTER_NEAREST, VK_FILTER_NEAREST);


	Core::Clock clock;
	Vulkan::DescriptorSet textureDescriptorSet = pipeline.AllocateDescriptorSet(0);


	while (!window.CloseRequested())
	{
		Window::PollInput();

		while (auto event = window.NextEvent())
		{
			if (auto text = event->Value<Window::Events::Text>())
			{
				std::u8string c = Core::ToUTF8(text->codepoint).Unwrap();
				std::cout << (const char*) c.data() << std::endl;
			}
		}


		Core::Math::Mat4f MVPMatrix;
		Core::Math::Vec3f Color((std::sin(*clock) + 1.0f) / 2.0f,
								(std::cos(0.25 * *clock) + 1.0f) / 2.0f,
								(std::cos(0.5 * *clock) + 1.0f) / 2.0f);


		textureDescriptorSet.SetUniformTexture(sampler, textureView, VK_IMAGE_LAYOUT_GENERAL, 0);


		commandBuffer.Begin(true);
		commandBuffer.BeginRenderPass(renderPass, framebuffer);
		commandBuffer.BindPipeline(pipeline);
		commandBuffer.BindVertexBuffer(0, buffer);
		commandBuffer.BindDescriptorSet(pipeline, 0, textureDescriptorSet);
		commandBuffer.PushConstants(pipeline, VK_SHADER_STAGE_VERTEX_BIT, Core::IO::DynamicByteBuffer(MVPMatrix), 0);
		commandBuffer.PushConstants(pipeline, VK_SHADER_STAGE_FRAGMENT_BIT, Core::IO::DynamicByteBuffer(Color), 64);
		commandBuffer.Draw(6);
		commandBuffer.EndRenderPass();
		commandBuffer.End();
		queue->Submit(commandBuffer);
		queue->WaitUntilIdle();


		swapchain.Present(framebuffer);
		window.SwapBuffers();
	}
}

int main()
{
	BasicRendering();
	return 0;
}
