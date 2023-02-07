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

	void DynamicSystemAssembler::LinkBody(Entity focus, Entity target, const glm::dvec2& focusLocal, const glm::dvec2& targetLocal)
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;

		Ref<Enyoo::LinkConstraint> linkConstraint = CreateRef<Enyoo::LinkConstraint>();
		m_Scene->GetRigidBodySystem()->AddConstraint(linkConstraint);
		m_Scene->GetRigidBodySystem()->AddRigidBody(focusBody.get());
		linkConstraint->SetFirstBody(focusBody.get());
		linkConstraint->SetSecondBody(targetBody.get());
		linkConstraint->SetFirstBodyLocal(focusLocal);
		linkConstraint->SetSecondBodyLocal(targetLocal);
    }

	void DynamicSystemAssembler::FixBody(Entity focus, Entity target, const glm::dvec2& local)
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		Ref<Enyoo::FixedPositionConstraint> fixedPositionConstraint = CreateRef<Enyoo::FixedPositionConstraint>();
		glm::dvec2 world = focusBody->LocalToWorld(local);
		m_Scene->GetRigidBodySystem()->AddConstraint(fixedPositionConstraint);
		fixedPositionConstraint->SetBody(focusBody.get());
		fixedPositionConstraint->SetLocalPosition(local);
		fixedPositionConstraint->SetWorldPosition(world);
	}

    bool DynamicSystemAssembler::GenerateRigidBodies()
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
			body->Mass = transform.Scale.x * rbc.Density;
			body->MomentInertia = (1.0 / 12.0) * body->Mass * transform.Scale.x * transform.Scale.x; //for right now we are assuming pretty uniform rectangles
			body->Fixed = rbc.Fixed;

			rbc.RuntimeBody = body;
		}

        return false;
    }

	bool DynamicSystemAssembler::GenerateConstraints()
	{
		auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent, LinkPointsComponent>();
		auto allBodies = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();

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

				//iterate over a list of all other link points some how
				auto LinkMap = m_Scene->GetLinkPointMap();
				for (auto iter = LinkMap.begin(); iter != LinkMap.end(); iter++)
				{
					if (iter->first == it->first)
						continue;

					//if both parties have a link in the adj matrix then continue
					Entity target = m_Scene->GetEntity(iter->first);

					auto& targetTransform = target.GetComponent<TransformComponent>();
					glm::dvec2 targetWorld = Enyoo::Utilities::LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

					if (std::fabs(targetWorld.x - focusWorld.x) < 0.1 && std::fabs(targetWorld.y - focusWorld.y) < 0.1)
							m_AdjacencyList.emplace(focus, target);
							
				}
			}
		}

		std::queue<Entity> leafNodes;
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			auto& rbc = entity.GetComponent<RigidBodyComponent>();
			auto& focusTransform = entity.GetComponent<TransformComponent>();
			uint32_t linkCount = 0;

			auto currentLinkPoints = m_Scene->GetLinkPoints(entity.GetUUID());
			for (auto it = currentLinkPoints.first; it != currentLinkPoints.second; it++)
			{
				auto LinkMap = m_Scene->GetLinkPointMap();
				for (auto iter = LinkMap.begin(); iter != LinkMap.end(); iter++)
				{
					Entity otherEntity = m_Scene->GetEntity(iter->first);

					if (entity == otherEntity)
						continue;

					auto& targetTransform = otherEntity.GetComponent<TransformComponent>();
					auto focusWorld = Enyoo::Utilities::LocalToWorld(it->second, focusTransform.Rotation, focusTransform.Translation);
					auto targetWorld = Enyoo::Utilities::LocalToWorld(iter->second, targetTransform.Rotation, targetTransform.Translation);

					if (std::fabs(targetWorld.x - focusWorld.x) < 0.1 && std::fabs(targetWorld.y - focusWorld.y) < 0.1)
					{
						linkCount++;
						break;
					}
				}
			}

			if (linkCount == 1 && !rbc.Fixed) 
				leafNodes.push(entity);

			//if (m_AdjacencyList.count(entity) == 0)
			//{
			//	m_Scene->GetRigidBodySystem()->AddRigidBody(rbc.RuntimeBody.get()); //I dont think our phys engine supports non linked bodies
			//}
		}


		while (!leafNodes.empty())
		{
			Entity entity = leafNodes.front();
			leafNodes.pop();
			auto path = BFSGeneratePath(m_AdjacencyList, entity);

			Entity focusNode = entity;
			for (auto targetNode : path)
			{
				if (focusNode == targetNode) 
					continue;

				if (IsInPath(this->m_HandledEntities, focusNode)) 
					continue;

				auto& focusBody = focusNode.GetComponent<RigidBodyComponent>();
				auto& targetBody = targetNode.GetComponent<RigidBodyComponent>();
				auto [focusLocal, targetLocal] = GetLocals(focusNode, targetNode);

				if (targetBody.Fixed) 
				{
					m_Scene->GetRigidBodySystem()->AddRigidBody(focusBody.RuntimeBody.get());
					m_HandledEntities.push_back(focusNode);
					break;
				}

				LinkBody(focusNode, targetNode, focusLocal, targetLocal);
				m_HandledEntities.push_back(focusNode);
				focusNode = targetNode;
			}


		}

		for (auto e : allBodies)
		{
			Entity entity{ e, m_Scene };
			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			if (m_AdjacencyList.count(entity) == 0)
			{
				m_Scene->GetRigidBodySystem()->AddRigidBody(rbc.RuntimeBody.get());
			}
		}

		for (auto e : view)
		{
			Entity entity{ e, m_Scene };

			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			if (rbc.Fixed)
			{
				auto range = m_AdjacencyList.equal_range(entity);

				for (auto it = range.first; it != range.second; it++)
				{
					if (entity == it->second)
						HZ_CORE_ASSERT(false, "this should never happen");

					auto [focusLocal, targetLocal] = GetLocals(it->second, entity);

					FixBody(it->second, entity, focusLocal);
				}
			}
		}

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
					m_Scene->GetRigidBodySystem()->AddForceGen(gravGen.get());
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

	std::pair<glm::dvec2, glm::dvec2> DynamicSystemAssembler::GetLocals(Entity focusEntity, Entity targetEntity)
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

	bool DynamicSystemAssembler::IsInPath(const std::vector<Entity>& path, Entity entity)
	{
		for (auto e : path)
		{
			if (e == entity)
				return true;
		}

		return false;
	}

	std::vector<Entity> DynamicSystemAssembler::BFSGeneratePath(const std::unordered_multimap<Entity, Entity>& graph, Entity source)
	{
		std::vector<Entity> currentPath;
		std::queue<std::vector<Entity>> queuedPaths;
		currentPath.push_back(source);
		queuedPaths.push(currentPath);
	
		while (!queuedPaths.empty())
		{
			currentPath = queuedPaths.front();
			queuedPaths.pop();
	
			Entity lastEntityInPath = currentPath.back();
		
			if (lastEntityInPath.GetComponent<RigidBodyComponent>().Fixed) 
				return currentPath;
	
			auto range = graph.equal_range(lastEntityInPath);
			for (auto it = range.first; it != range.second; ++it)
			{
				if (!IsInPath(currentPath, it->second))
				{
					std::vector<Entity> newPath(currentPath.begin(), currentPath.end());
					newPath.push_back(it->second);
					queuedPaths.push(newPath);
				}
			}
		}

		return currentPath;
	}
}
