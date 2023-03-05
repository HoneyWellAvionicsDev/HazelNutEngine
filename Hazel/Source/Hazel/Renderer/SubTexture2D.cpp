#include "hzpch.h"
#include "SubTexture2D.h"

namespace Jbonk
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
		: m_Texture(texture)
	{
		m_TextureCoords[0] = { min.x, min.y };
		m_TextureCoords[1] = { max.x, min.y };
		m_TextureCoords[2] = { max.x, max.y };
		m_TextureCoords[3] = { min.x, max.y };
	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& spriteCellDimensions, const glm::vec2& spriteCellSize)
	{
		glm::vec2 min = { (coords.x * spriteCellDimensions.x) / texture->GetWidth(), (coords.y * spriteCellDimensions.y) / texture->GetHeight() };
		glm::vec2 max = { ((coords.x + spriteCellSize.x) * spriteCellDimensions.x) / texture->GetWidth(), ((coords.y + spriteCellSize.y) * spriteCellDimensions.y) / texture->GetHeight() };
		return CreateRef<SubTexture2D>(texture, min, max);
	}

}