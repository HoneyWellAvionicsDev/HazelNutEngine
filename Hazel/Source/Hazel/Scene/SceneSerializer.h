#pragma once

#include "Scene.h"

namespace YAML
{ 
	class Emitter;
}

namespace Jbonk
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::filesystem::path& filepath);
		void SerializeRuntime(const std::filesystem::path& filepath);
		void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::filesystem::path& filepath);
		bool DeserializeRuntime(const std::filesystem::path& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}

