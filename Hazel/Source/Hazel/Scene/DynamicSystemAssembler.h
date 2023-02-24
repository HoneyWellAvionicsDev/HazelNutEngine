#pragma once

#include "Entity.h"

#include "Hazel/Math/Matrix.h"

#include <map>
#include <queue>

namespace Hazel
{
	static constexpr float s_LinkPointRadius = 0.1;

	struct Joint
	{
		Entity Ent1;
		Entity Ent2;
		glm::dvec2 WorldPoint;

		Joint(Entity focus, Entity target, const glm::dvec2& worldPoint)
			: Ent1(focus), Ent2(target), WorldPoint(worldPoint) {}
	};

	class Scene;

	class DynamicSystemAssembler
	{
	public:
		using LocalPoints = std::pair<glm::dvec2, glm::dvec2>;
		using EntityView = entt::basic_view<entt::entity, entt::get_t<RigidBodyComponent, LinkPointsComponent>, entt::exclude_t<>, void>;
		using AdjacencyList = std::unordered_multimap<Entity, Entity>;

		DynamicSystemAssembler(Scene* scene, EntityView entityView);

		void CreateLinkConstraint(Entity focus, Entity target);
		void CreateFixedConstraint(Entity focus, Entity target, const glm::dvec2& focusLocal);
		[[NODISCARD]] Ref<Enyoo::Spring> CreateSpring(Entity endBody1, Entity endBody2, const glm::dvec2& body1Local, const glm::dvec2& body2Local);

		void GenerateRigidBodies();
		void GenerateConstraints();
		bool GenerateForceGens();

		LocalPoints GetMatchingLocals(Entity focusEntity, Entity targetEntity) const;
		bool Adjacent(Entity focus, Entity target) const;

	private:
		void CreateSpringForce(Entity entity);
		bool FixedBody(Entity entity) const;
		bool SpringBody(Entity entity) const;
		bool Proximity(const glm::dvec2& focusWorld, const glm::dvec2& targetWorld) const;
		size_t HashJoint(const Joint& joint) const;
		Entity FindAdjacentFixedBody(Entity entity) const;
		Entity FindBody(const glm::dvec2& focusWorld, UUID uuid) const;
		void GenerateAdjacencyList(const EntityView& view);
		void HandleFixedBodies(const EntityView& view);
		void HandleLinkedBodies(const EntityView& view);
	private:
		Scene* m_Scene = nullptr;
		AdjacencyList m_AdjacencyList;
		EntityView m_EntityView;
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
*
* 
* 
* Weird observations / limitations:
* Really massive bodies cause system instability with link and fixed constraints (they will start spinning and system will not converge)
* This also happens upon applying really large forces
* Two bodies between two fixed points that should be linked to each other will not
* If you place a third body between the two linked bodies then the link will create but IF the original two bodies are not at an angle, then the system does not converge
* 
* Things I wanna do with this class:
* 1. Handle springs
* 2. Handle other constraints
*/
