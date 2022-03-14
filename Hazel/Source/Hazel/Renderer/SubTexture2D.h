#pragma once

#include <glm/glm.hpp>
#include "Texture.h"

namespace Hazel
{
	class SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		const Ref<Texture2D> GetTexture() const { return m_Texture; }
		const glm::vec2* GetTextureCoords() { return m_TextureCoords; }
		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& spriteCellDimensions, const glm::vec2& spriteCellSize = { 1, 1 });
	private:
		Ref<Texture2D> m_Texture;

		glm::vec2 m_TextureCoords[4];
	};
}

