#pragma once

#include "Hazel/Renderer/Texture.h"

#include <filesystem>

namespace Hazel
{
	class ContentViewPanel
	{
	public:
		ContentViewPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
