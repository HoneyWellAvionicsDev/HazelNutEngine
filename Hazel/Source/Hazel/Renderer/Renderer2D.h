#pragma once

#include "OrthographicCamera.h"
#include "Texture.h"
#include "SubTexture2D.h"

namespace Hazel
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

		//rotation accepts angle in radians by default
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const Ref<SubTexture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void StartNewBatch();
	};
}

