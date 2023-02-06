#include "hzpch.h"

#include "Hazel/Physics/RigidBodySystem.h"

#include "DynamicSystemAssembler.h"
#include "Components.h"

#include "Entity.h"


namespace Hazel
{
	std::pair<glm::dvec2, glm::dvec2> GetLocals(Entity focusEntity, Entity targetEntity, Scene* scene);
	bool IsInPath(const std::vector<Entity>& path, Entity entity);
	std::vector<Hazel::Entity> BreadthFirstSearch(const std::unordered_multimap<Hazel::Entity, Hazel::Entity>& graph, Hazel::Entity source);

    DynamicSystemAssembler::DynamicSystemAssembler(Scene* scene)
        : m_Scene(scene)
    {
    }

	void DynamicSystemAssembler::LinkBody(Entity focus, Entity target, const glm::dvec2& focusLocal, const glm::dvec2& targetLocal)
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
    }

	void DynamicSystemAssembler::FixBody(Entity focus, Entity target, const glm::dvec2& world)
	{
		auto& focusBody = focus.GetComponent<RigidBodyComponent>().RuntimeBody;
		auto& targetBody = target.GetComponent<RigidBodyComponent>().RuntimeBody;
		Ref<Enyoo::FixedPositionConstraint> fixedPositionConstraint = CreateRef<Enyoo::FixedPositionConstraint>();
		glm::dvec2 local = focusBody->WorldToLocal(world);
		m_Scene->m_NewBodySystem->AddConstraint(fixedPositionConstraint);
		fixedPositionConstraint->SetBody(focusBody.get());
		fixedPositionConstraint->SetLocalPosition(local);
		fixedPositionConstraint->SetWorldPosition(world);
	}

    bool DynamicSystemAssembler::GenerateRigidBodies()
    {
        auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
		size_t Index = 0;
	
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
				auto LinkMap = m_Scene->m_EntityLinkPointMap;
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

		for (auto it = m_AdjacencyList.begin(); it != m_AdjacencyList.end(); it++)
		{
			auto handle = it->first.GetHandle();
			auto& IDC = m_Scene->m_Registry.get<IDComponent>(handle);
			HZ_CORE_TRACE("First: '{0}'", IDC.ID);
			HZ_CORE_TRACE("Second: '{0}'", it->second.GetUUID());
		}

		std::queue<Entity> leafNodes;
		for (auto e : view)
		{
			Entity entity{ e, m_Scene };
			auto& rbc = entity.GetComponent<RigidBodyComponent>();

			if (m_AdjacencyList.count(entity) == 1) //this check is not sufficent
			{
				//ensure entity is not a fixed rigidbody
				if (rbc.Fixed)
					continue;

				leafNodes.push(entity);
			}
		}


		while (!leafNodes.empty())
		{
			Entity entity = leafNodes.front();
			HZ_CORE_TRACE("QUEUE: {0}", entity.GetUUID());
			leafNodes.pop();
			auto path = BreadthFirstSearch(m_AdjacencyList, entity);

			Entity focusNode = entity;
			for (auto targetNode : path)
			{
				if (focusNode == targetNode) //avoid self linkage 
					continue;

				if (IsInPath(this->m_HandledEntities, focusNode)) //avoid linking if focusNode has been linked already
					continue;

				auto& focusBody = focusNode.GetComponent<RigidBodyComponent>();
				auto& targetBody = targetNode.GetComponent<RigidBodyComponent>();
				auto [focusLocal, targetLocal] = GetLocals(focusNode, targetNode, m_Scene);

				if (targetBody.Fixed)
				{
					m_Scene->m_NewBodySystem->AddRigidBody(focusBody.RuntimeBody.get());
					break;
				}

				LinkBody(focusNode, targetNode, focusLocal, targetLocal);

				HZ_CORE_TRACE("ADDED {0}", focusNode.GetUUID());
				m_HandledEntities.push_back(focusNode);

				focusNode = targetNode;
			}


		}
		//do fixed constraints seperately


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

	std::pair<glm::dvec2, glm::dvec2> GetLocals(Entity focusEntity, Entity targetEntity, Scene* scene)
	{
		auto focusRange = scene->GetLinkPoints(focusEntity.GetUUID());
		auto targetRange = scene->GetLinkPoints(targetEntity.GetUUID());
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

	bool IsInPath(const std::vector<Entity>& path, Entity entity)
	{
		for (auto e : path)
		{
			if (e == entity)
				return true;
		}

		return false;
	}

	std::vector<Hazel::Entity> BreadthFirstSearch(const std::unordered_multimap<Hazel::Entity, Hazel::Entity>& graph, Hazel::Entity source)
	{
		std::vector<Hazel::Entity> currentPath;
		std::queue<std::vector< Hazel::Entity>> queuedPaths;
		std::unordered_map<Hazel::Entity, bool> visited;
		currentPath.push_back(source);
		queuedPaths.push(currentPath);
	
		while (!queuedPaths.empty())
		{
			currentPath = queuedPaths.front();
			queuedPaths.pop();
	
			Hazel::Entity lastEntityInPath = currentPath.back();
			// Check if the current node has already been visited
			//if (visited[lastEntityInPath])
			//	continue;
	
			// Mark the node as visited
			visited[lastEntityInPath] = true;
	
			if (lastEntityInPath.GetComponent<RigidBodyComponent>().Fixed) //terminate condition
			{
	
				return currentPath;
			}
	
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
