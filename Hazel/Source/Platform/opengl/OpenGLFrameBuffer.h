#pragma once

#include "Hazel/Renderer/FrameBuffer.h"

namespace Hazel
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& specs);
		virtual ~OpenGLFrameBuffer();
		void Invalidate();
		void Bind() override;
		void Unbind() override;
		uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FrameBufferSpecification m_Specification;
		

		
	};
}

