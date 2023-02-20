#pragma once

#include "Entity.h"

#include "Hazel/Math/Matrix.h"

#include <map>
#include <queue>

namespace Hazel
{
	struct Joint
	{
		Entity Ent1;
		Entity Ent2;
		glm::dvec2 WorldPoint;

		Joint(Entity focus, Entity target, const glm::dvec2& worldPoint)
			: Ent1(focus), Ent2(target), WorldPoint(worldPoint) {}
	};

	static size_t HashJoint(const Joint& joint)
	{
		size_t seed = 0;
		Entity ent1{ joint.Ent1.GetHandle(), joint.Ent1.GetScene() };
		Entity ent2{ joint.Ent2.GetHandle(),  joint.Ent2.GetScene() };

		size_t hash1 = static_cast<size_t>(ent1.GetUUID() * ent2.GetUUID());
		size_t hash2 = static_cast<size_t>(joint.WorldPoint.x * joint.WorldPoint.y);
		seed ^= hash1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}

	class Scene;

	class DynamicSystemAssembler
	{
	public:
		using LocalPoints = std::pair<glm::dvec2, glm::dvec2>;
		using EntityView = entt::basic_view<entt::entity, entt::get_t<RigidBodyComponent, LinkPointsComponent>, entt::exclude_t<>, void>;
		using AllBodiesView = entt::basic_view<entt::entity, entt::get_t<RigidBodyComponent>, entt::exclude_t<>, void>;
		using AdjacencyList = std::unordered_multimap<Entity, Entity>;

		DynamicSystemAssembler(Scene* scene);

		void CreateLinkConstraint(Entity focus, Entity target);
		void CreateFixedConstraint(Entity focus, Entity target, const glm::dvec2& focusLocal);
		void CreateSpringForce(Entity endBody1, Entity endBody2, const glm::dvec2& body1Local, const glm::dvec2& body2Local, double restLength);

		void GenerateRigidBodies();
		bool GenerateConstraints();
		bool GenerateForceGens();

		LocalPoints GetMatchingLocals(Entity focusEntity, Entity targetEntity);
		bool Adjacent(Entity focus, Entity target);
		bool FixedBody(Entity entity) const;
		bool Handled(Entity entity) const;
		bool Close(const glm::dvec2& focusWorld, const glm::dvec2& targetWorld);
		
	private:
		void GenerateAdjacencyList(const EntityView& view);
		void HandleFixedBodies(const EntityView& view);
		void HandleLinkedBodies(const EntityView& view);
	private:
		Scene* m_Scene = nullptr;
		AdjacencyList m_AdjacencyList;
		std::unordered_set<Entity> m_HandledBodies;
		std::unordered_set<size_t> m_Joints;
		std::unordered_set<Entity> m_Fixed;
	};
}
/* option 1
*
* option 2
* def joint: the world coord of the two link points from two adjacent bodies
* goal: no duplicate joints between the same two bodies
* everytime we link a focus to a target we will create a hash from focus * target + the joint (be sure to test this hash func so that target * focus + joint creates the same hash
* check that this hash is not in the set and that target body is not a spring
* if not then link both bodies and push the hash into the set
* otherwise move on to the next adjacent body from the focus body
* 
* for now it would still be a good idea to handle fixed bodies first and fix their neighbors
* when we visit a rigid body like a spring we will want to only create one spring object from the first two link points found. All others will be ignored (handle springs last)
*/
