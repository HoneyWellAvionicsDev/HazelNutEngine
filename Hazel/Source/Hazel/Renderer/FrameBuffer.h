#pragma once

#include "Hazel/Core/Core.h"


namespace Hazel
{
	enum class FrameBufferTextureFormat
	{
		None = 0,

		//Color
		RGBA8,
		RED_INTEGER,

		//DEPTH/stencil
		DEPTH24STENCIL8,

		//Default
		Depth = DEPTH24STENCIL8
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format)
			:TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		//TODO: filtering/wrap
	};

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments)
			:Attachments(attachments) {}

		std::vector<FrameBufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification
	{
		uint32_t Width, Height;
		FrameBufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, /*const void**/int value) = 0; //maybe template this later to specifiy type

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& specs);
		virtual void Test() = 0; //remove this
	};
}

