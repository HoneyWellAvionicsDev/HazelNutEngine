#pragma once

#include "Hazel/Math/Matrix.h"

namespace Hazel
{
	class Scene;
	class Entity;

	class DynamicSystemAssembler
	{
	public:
		DynamicSystemAssembler(Scene* scene);

		bool LinkBody(Entity focus, Entity target, const glm::dvec2& focusWorld, const glm::dvec2& targetWorld);
		bool FixBody(Entity focus, Entity target, const glm::dvec2& world);

		bool GenerateRigidBodies();
		bool GenerateConstraints();
		bool GenerateForceGens();

	private:
		Scene* m_Scene = nullptr;
		std::unordered_map<Enyoo::RigidBody*, size_t> m_BodyMap;
		Math::Matrix m_AdjacencyMatrix;
	};
}