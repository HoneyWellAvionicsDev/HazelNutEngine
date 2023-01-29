#include "hzpch.h"

#include "Hazel/Physics/RigidBodySystem.h"

#include "DynamicSystemAssembler.h"
#include "Components.h"

#include "Entity.h"

namespace Hazel
{
    DynamicSystemAssembler::DynamicSystemAssembler(Scene* scene)
        : m_Scene(scene)
    {
    }

    bool DynamicSystemAssembler::LinkBody(Entity focus, Entity target, const glm::dvec2& focusLocal, const glm::dvec2& targetLocal)
    {
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;

		Ref<Enyoo::LinkConstraint> linkConstraint = CreateRef<Enyoo::LinkConstraint>();
		m_Scene->m_NewBodySystem->AddConstraint(linkConstraint);
		m_Scene->m_NewBodySystem->AddRigidBody(focusBody.get());
		linkConstraint->SetFirstBody(focusBody.get());
		linkConstraint->SetSecondBody(targetBody.get());
		linkConstraint->SetFirstBodyLocal(focusLocal);
		linkConstraint->SetSecondBodyLocal(targetLocal);

		size_t i = m_BodyMap.at(focusBody.get());
		size_t j = m_BodyMap.at(targetBody.get());
		m_AdjacencyMatrix[i][j] = m_AdjacencyMatrix[j][i] = 1.0;

		//HZ_CORE_TRACE("LINKED");

        return false;
    }

	bool DynamicSystemAssembler::FixBody(Entity focus, Entity target, const glm::dvec2& world)
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		float x = world.x;
		float y = world.y;
		Ref<Enyoo::FixedPositionConstraint> fixedPositionConstraint = CreateRef<Enyoo::FixedPositionConstraint>();
		glm::dvec2 local = focusBody->WorldToLocal(world);
		m_Scene->m_NewBodySystem->AddConstraint(fixedPositionConstraint);
		m_Scene->m_NewBodySystem->AddRigidBody(focusBody.get());
		fixedPositionConstraint->SetBody(focusBody.get());
		fixedPositionConstraint->SetLocalPosition(local);
		fixedPositionConstraint->SetWorldPosition(world);

		size_t i = m_BodyMap.at(focusBody.get());
		size_t j = m_BodyMap.at(targetBody.get());
		m_AdjacencyMatrix[i][j] = m_AdjacencyMatrix[j][i] = 1.0;

		//HZ_CORE_TRACE("FIXED");

		return false;
	}

    bool DynamicSystemAssembler::GenerateRigidBodies()
    {
        auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
		m_BodyMap.clear();
		size_t Index = 0;
	
		for (auto e : view)											//auto ie = view.rbegin(); ie != view.rend(); ie++
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			Ref<Enyoo::RigidBody> body = CreateRef<Enyoo::RigidBody>();
			body->Position = { transform.Translation.x, transform.Translation.y };
			body->Theta = transform.Rotation.z;
			body->Velocity = glm::dvec2{ 0.0 };
			body->AngularVelocity = 0.0;
			body->Mass = transform.Scale.x * rbc.Density;
			body->MomentInertia = (1.0 / 12.0) * body->Mass * transform.Scale.x * transform.Scale.x; //for right now we are assuming pretty uniform rectangles

			if (entity.HasComponent<LinkPointsComponent>())
				m_BodyMap[body.get()] = Index++;


			rbc.RuntimeBody = body;
		}

		m_AdjacencyMatrix.Initialize(Index + 1, Index + 1);

        return false;
    }

	bool DynamicSystemAssembler::GenerateConstraints()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent, LinkPointsComponent>();

		for (auto e : view)
		{
			Entity focus = { e, m_Scene };
			auto& focusTransform = focus.GetComponent<TransformComponent>();
			auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
			uint32_t constraintCount = 0;
			bool bodyFixed = false;

			if (focus.GetComponent<RigidBodyComponent>().Fixed)
			{
				m_Scene->m_NewBodySystem->AddRigidBody(focusBody.get());
				continue;
			}

			auto range = m_Scene->GetLinkPoints(focus.GetUUID());
			for (auto it = range.first; it != range.second; it++)
			{
				glm::dvec2 focusWorld = Enyoo::Utilities::LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);

				//iterate over a list of all other link points some how
				auto LinkMap = m_Scene->m_EntityLinkPointMap;
				for (auto iter = LinkMap.begin(); iter != LinkMap.end(); iter++)
				{
					if (iter->first == it->first)
						continue;

					//if both parties have a link in the adj matrix then continue
					Entity target = m_Scene->GetEntity(iter->first);
					auto& targetBody = target.GetComponent<RigidBodyComponent>();
					size_t i = m_BodyMap.at(focusBody.get());
					size_t j = m_BodyMap.at(targetBody.RuntimeBody.get());
					if (m_AdjacencyMatrix[i][j] == 1.0 || m_AdjacencyMatrix[j][i] == 1.0)
						continue;

					auto& targetTransform = target.GetComponent<TransformComponent>();
					glm::dvec2 targetWorld = Enyoo::Utilities::LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

					if (std::fabs(targetWorld.x - focusWorld.x) < 0.1 && std::fabs(targetWorld.y - focusWorld.y) < 0.1)
					{
						//link
						
						//targetBody.Fixed ? FixBody(focus, target, iter->second) : LinkBody(focus, target, it->second, iter->second);
						if (targetBody.Fixed)
						{
							//FixBody(focus, target, targetWorld);
							//HZ_CORE_TRACE("FIXED");
							//HZ_CORE_TRACE("linked '{0}'", focus.GetUUID());
							//HZ_CORE_TRACE("linked '{0}'", target.GetUUID());
							//bodyFixed = true;
						}
						else
						{
							//if(!bodyFixed && !(focus.GetUUID() == 15919350632137829342))
								LinkBody(focus, target, it->second, iter->second);
							HZ_CORE_TRACE("LINKED");
							HZ_CORE_TRACE("linked '{0}'", focus.GetUUID());
							HZ_CORE_TRACE("linked '{0}'", target.GetUUID());
						}
						constraintCount++;
					}
				}
			}

			if (!constraintCount)
				m_Scene->m_NewBodySystem->AddRigidBody(focusBody.get());

		}

		m_AdjacencyMatrix.Print();

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
					Ref<Enyoo::GravitationalAccelerator> gravGen = CreateRef<Enyoo::GravitationalAccelerator>();
					gravGen->SetGravity(fgc.LocalGravity);
					m_Scene->m_NewBodySystem->AddForceGen(gravGen.get());
					fgc.RuntimeGenerator = gravGen;
					break;
				}
				case ForceGeneratorComponent::GeneratorType::Test1:
				{

				}
				case ForceGeneratorComponent::GeneratorType::Test2:
				{

				}
			}
		}

		return false;
	}
}
