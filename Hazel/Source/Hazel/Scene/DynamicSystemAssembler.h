#pragma once

#include "Entity.h"

#include "Hazel/Math/Matrix.h"

#include <map>
#include <queue>

namespace Hazel
{
	class Scene;

	class DynamicSystemAssembler
	{
	public:
		DynamicSystemAssembler(Scene* scene);

		void LinkBody(Entity focus, Entity target, const glm::dvec2& focusWorld, const glm::dvec2& targetWorld);
		void FixBody(Entity focus, Entity target, const glm::dvec2& world);

		bool GenerateRigidBodies();
		bool GenerateConstraints();
		bool GenerateForceGens();

		std::pair<glm::dvec2, glm::dvec2> GetLocals(Entity focusEntity, Entity targetEntity);
		bool IsInPath(const std::vector<Entity>& path, Entity entity);
		std::vector<Hazel::Entity> BFSGeneratePath(const std::unordered_multimap<Hazel::Entity, Hazel::Entity>& graph, Hazel::Entity source);


	private:
		Scene* m_Scene = nullptr;
		std::unordered_multimap<Entity, Entity> m_AdjacencyList;
		std::vector<Entity> m_HandledEntities;
	};
}