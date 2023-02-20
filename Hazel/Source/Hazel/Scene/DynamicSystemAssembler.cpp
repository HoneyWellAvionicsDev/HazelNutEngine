#include "hzpch.h"

#include "DynamicSystemAssembler.h"
#include "Components.h"
#include "Entity.h"

#include "Hazel/Physics/RigidBodySystem.h"


namespace Hazel
{
	DynamicSystemAssembler::DynamicSystemAssembler(Scene* scene)
		: m_Scene(scene)
	{
	}

	void DynamicSystemAssembler::CreateLinkConstraint(Entity focus, Entity target)
	{
		HZ_CORE_ASSERT(focus != target);

		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto [focusLocal, targetLocal] = GetMatchingLocals(focus, target);
		glm::dvec2 world = targetBody->LocalToWorld(targetLocal);

		Joint joint = Joint(focus, target, world);
		size_t hash = HashJoint(joint);

		if (m_Joints.count(hash))
			return;

		Ref<Enyoo::LinkConstraint> linkConstraint = CreateRef<Enyoo::LinkConstraint>();
		linkConstraint->SetFirstBody(focusBody.get());
		linkConstraint->SetSecondBody(targetBody.get());
		linkConstraint->SetFirstBodyLocal(focusLocal);
		linkConstraint->SetSecondBodyLocal(targetLocal);

		m_Scene->GetRigidBodySystem()->AddConstraint(linkConstraint);
		m_Joints.insert(hash);

		HZ_CORE_TRACE("LINKED: {0} to {1} hash: {2}", focus.GetUUID(), target.GetUUID(), hash);
	}

	void DynamicSystemAssembler::CreateFixedConstraint(Entity focus, Entity target, const glm::dvec2& focusLocal)
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;

		Ref<Enyoo::FixedPositionConstraint> fixedPositionConstraint = CreateRef<Enyoo::FixedPositionConstraint>();
		glm::dvec2 world = focusBody->LocalToWorld(focusLocal);
		fixedPositionConstraint->SetBody(focusBody.get());
		fixedPositionConstraint->SetLocalPosition(focusLocal);
		fixedPositionConstraint->SetWorldPosition(world);
		m_Scene->GetRigidBodySystem()->AddConstraint(fixedPositionConstraint);
	}

	void DynamicSystemAssembler::CreateSpringForce(Entity endBody1, Entity endBody2, const glm::dvec2& body1Local, const glm::dvec2& body2Local, double restLength)
	{
		auto& focusBody = endBody1.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = endBody2.GetComponent<RigidBodyComponent>().RuntimeBody;

		Ref<Enyoo::Spring> springForceGenerator = CreateRef<Enyoo::Spring>();
		springForceGenerator->SetFirstBody(focusBody.get());
		springForceGenerator->SetSecondBody(targetBody.get());
		springForceGenerator->SetFirstPosition(body1Local);
		springForceGenerator->SetSecondPosition(body2Local);
		springForceGenerator->SetRestLength(restLength);
		m_Scene->GetRigidBodySystem()->AddForceGen(springForceGenerator);
	}

	void DynamicSystemAssembler::GenerateRigidBodies()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();

		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			Ref<Enyoo::RigidBody> body = CreateRef<Enyoo::RigidBody>();
			body->Position = { transform.Translation.x, transform.Translation.y };
			body->Theta = transform.Rotation.z;
			body->Velocity = glm::dvec2{ 0.0 };
			body->AngularVelocity = 0.0;
			body->Mass = transform.Scale.x * rbc.Density; //*scale.y
			body->MomentInertia = (1.0 / 12.0) * body->Mass * transform.Scale.x * transform.Scale.x; //for right now we are assuming pretty uniform rectangles
			body->Fixed = rbc.Fixed;

			rbc.RuntimeBody = body;
			m_Scene->GetRigidBodySystem()->AddRigidBody(body);
		}
	}

	bool DynamicSystemAssembler::GenerateConstraints()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent, LinkPointsComponent>();
		auto allBodies = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
	
		GenerateAdjacencyList(view);
		HandleFixedBodies(view);
		HandleLinkedBodies(view);

		return false;
	}

	bool DynamicSystemAssembler::GenerateForceGens()
	{
		auto view = m_Scene->GetAllEntitiesWith<ForceGeneratorComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& fgc = entity.GetComponent<ForceGeneratorComponent>();

			switch (fgc.Type)
			{
				case ForceGeneratorComponent::GeneratorType::Gravity:
				{
					Ref<Enyoo::LocalGravity> gravGen = CreateRef<Enyoo::LocalGravity>();
					gravGen->SetGravity(fgc.LocalGravity);
					m_Scene->GetRigidBodySystem()->AddForceGen(gravGen);
					fgc.RuntimeGenerator = gravGen;
					break;
				}
				case ForceGeneratorComponent::GeneratorType::GravitationalAccelerator:
				{
					Ref<Enyoo::GravitationalAccelerator> gravSphere = CreateRef<Enyoo::GravitationalAccelerator>();
					auto& body = entity.GetComponent<RigidBodyComponent>().RuntimeBody;
					gravSphere->SetSourceBody(body.get());
					gravSphere->SetInfluenceRadius(200.0);
					gravSphere->SetRepulisveForce(fgc.RepulsiveForce);
					m_Scene->GetRigidBodySystem()->AddForceGen(gravSphere);
					fgc.RuntimeGenerator = gravSphere;
					break;
				}
				case ForceGeneratorComponent::GeneratorType::Spring:
				{
					
				}
			}
		}

		return false;
	}

	bool DynamicSystemAssembler::Adjacent(Entity focus, Entity target)
	{
		auto range = m_AdjacencyList.equal_range(target);

		for (auto it = range.first; it != range.second; it++)
		{
			if (it->second == focus)
				return true;
		}

		return false;
	}

	DynamicSystemAssembler::LocalPoints DynamicSystemAssembler::GetMatchingLocals(Entity focusEntity, Entity targetEntity)
	{
		auto focusRange = m_Scene->GetLinkPoints(focusEntity.GetUUID());
		auto targetRange = m_Scene->GetLinkPoints(targetEntity.GetUUID());
		auto& focusTransform = focusEntity.GetComponent<TransformComponent>();
		auto& targetTransform = targetEntity.GetComponent<TransformComponent>();

		for (auto it = focusRange.first; it != focusRange.second; it++)
		{
			for (auto iter = targetRange.first; iter != targetRange.second; iter++)
			{
				auto focusWorld = Enyoo::Utilities::LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);
				auto targetWorld = Enyoo::Utilities::LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

				if (std::fabs(targetWorld.x - focusWorld.x) < 0.1 && std::fabs(targetWorld.y - focusWorld.y) < 0.1)
					return { it->second, iter->second };
			}
		}

		HZ_CORE_ASSERT(false, "How did this happen?");
		return {};
	}

	bool DynamicSystemAssembler::FixedBody(Entity entity) const
	{
		return entity.GetComponent<RigidBodyComponent>().Fixed;
	}

	bool DynamicSystemAssembler::Handled(Entity entity) const
	{
		return m_HandledBodies.count(entity);
	}

	bool DynamicSystemAssembler::Close(const glm::dvec2& focusWorld, const glm::dvec2& targetWorld)
	{
		return std::fabs(targetWorld.x - focusWorld.x) < 0.1 && std::fabs(targetWorld.y - focusWorld.y) < 0.1;
	}

	void DynamicSystemAssembler::GenerateAdjacencyList(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity focus = { e, m_Scene };
			auto& focusTransform = focus.GetComponent<TransformComponent>();
			auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
			uint32_t constraintCount = 0;

			auto range = m_Scene->GetLinkPoints(focus.GetUUID());
			for (auto it = range.first; it != range.second; it++)
			{
				glm::dvec2 focusWorld = Enyoo::Utilities::LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);

				auto LinkMap = m_Scene->GetLinkPointMap();
				for (auto iter = LinkMap.begin(); iter != LinkMap.end(); iter++)
				{
					if (iter->first == it->first)
						continue;

					Entity target = m_Scene->GetEntity(iter->first);
					auto& targetTransform = target.GetComponent<TransformComponent>();
					glm::dvec2 targetWorld = Enyoo::Utilities::LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

					if (Close(focusWorld, targetWorld))
						m_AdjacencyList.emplace(focus, target);

				}
			}
		}
	}

	void DynamicSystemAssembler::HandleFixedBodies(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			if (!rbc.Fixed)
				continue;

			m_HandledBodies.insert(entity);
			auto range = m_AdjacencyList.equal_range(entity);
			
			for (auto it = range.first; it != range.second; it++)
			{
				if (entity == it->second)
					HZ_CORE_ASSERT(false, "this should never happen");
			
				auto [focusLocal, targetLocal] = GetMatchingLocals(it->second, entity);
				auto& body = it->second.GetComponent<RigidBodyComponent>().RuntimeBody;
			
				CreateFixedConstraint(it->second, entity, focusLocal);
				m_Fixed.insert(it->second);
			}
		}
	}

	void DynamicSystemAssembler::HandleLinkedBodies(const EntityView& view)
	{
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			if (Handled(entity))
				continue;

			auto [first, second] = m_AdjacencyList.equal_range(entity);
			for (auto it = first; it != second; it++)
			{
				if (Handled(it->second))
					continue;

				if (m_Fixed.count(it->second))
					continue;

				CreateLinkConstraint(entity, it->second);
			}
		}
	}
}