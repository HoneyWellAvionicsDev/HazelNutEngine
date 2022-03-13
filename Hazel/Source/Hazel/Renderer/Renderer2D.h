#pragma once

#include "OrthographicCamera.h"
#include "Texture.h"

namespace Hazel
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationInRadians, const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.f), float tileFactor = 1.f);

	};
}

