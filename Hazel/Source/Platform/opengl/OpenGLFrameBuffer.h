#pragma once

#include "Hazel/Renderer/FrameBuffer.h"

namespace Jbonk
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& specs);
		virtual ~OpenGLFrameBuffer();
		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, /*const void**/int value) override;

		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { JB_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of bounds"); return m_ColorAttachments[index]; }
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_DepthAttachment = 0;
		
		std::vector<uint32_t> m_ColorAttachments;
		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentSpecifications;
		FrameBufferSpecification m_Specification;
		FrameBufferTextureSpecification m_DepthAttachmentSpecification = FrameBufferTextureFormat::None;
		
	};
}

