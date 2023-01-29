#pragma once

#include "Scene.h"

namespace YAML
{ 
	class Emitter;
}

namespace Hazel
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);
		void SerializeEntity(YAML::Emitter& out, Entity entity);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}

