#include <hzpch.h>

#include "Hazel/Renderer/Renderer2D.h"

#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "DynamicSystemAssembler.h"

#include <glm/glm.hpp>

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include "box2d/b2_circle_shape.h"


namespace Jbonk
{
	static b2BodyType NativeRigidbody2DTypeToBox2D(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:    return b2_staticBody;
			case RigidBody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
			case RigidBody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
		}

		JB_CORE_ASSERT(false, "Unknown body type");
		return b2_staticBody;
	}

	Scene::Scene()
		: m_SceneName("Untitled")
	{

	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src,
		const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto e : view)
			{
				UUID uuid = src.get<IDComponent>(e).ID;
				entt::entity dstEnttID = enttMap.at(uuid);

				if (enttMap.find(uuid) == enttMap.end())
					continue;

				auto& component = src.get<Component>(e);
				dst.emplace_or_replace<Component>(dstEnttID, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, 
		const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> source)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = source->m_ViewportWidth;
		newScene->m_ViewportHeight = source->m_ViewportHeight;
		newScene->m_EntityLinkPointMap = source->m_EntityLinkPointMap;
		newScene->m_SceneName = source->m_SceneName;
		newScene->m_PhysicsDeltaT = source->m_PhysicsDeltaT;
		newScene->m_PhysicsSteps = source->m_PhysicsSteps;

		auto& srcSceneRegistry = source->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		
		for (auto it = idView.rbegin(); it != idView.rend(); it++)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(*it).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(*it).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = static_cast<entt::entity>(newEntity);
		}

		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Ref<Scene> Scene::CopyExclusive(Ref<Scene> source, Entity exclusion)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = source->m_ViewportWidth;
		newScene->m_ViewportHeight = source->m_ViewportHeight;
		newScene->m_EntityLinkPointMap = source->m_EntityLinkPointMap;
		newScene->m_SceneName = source->m_SceneName;
		newScene->m_PhysicsDeltaT = source->m_PhysicsDeltaT;
		newScene->m_PhysicsSteps = source->m_PhysicsSteps;

		auto& srcSceneRegistry = source->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();

		for (auto it = idView.rbegin(); it != idView.rend(); it++)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(*it).ID;
			if (uuid == exclusion.GetUUID())
				continue;

			const auto& name = srcSceneRegistry.get<TagComponent>(*it).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = static_cast<entt::entity>(newEntity);
		}

		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		CopyComponentIfExists(AllComponents{}, newEntity, entity);

		if (entity.HasComponent<LinkPointsComponent>())
		{
			auto range = GetLinkPoints(entity.GetUUID());
			std::vector<glm::vec2> toBeCopied;
			
			for (auto it = range.first; it != range.second; it++)
				toBeCopied.push_back(it->second);
			
			for (auto lp : toBeCopied)
				AddLinkPoint(newEntity.GetUUID(), lp);
		}

		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.GetHandle());
	}

	void Scene::OnRuntimeStart()
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.InstantiateScript)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}
			}
		});
		

		OnPhysics2DStart();
		OnPhysicsStart();
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysics2DStop();
		OnPhysicsStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
		OnPhysicsStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
		OnPhysicsStop();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		UpdateScripts(ts);
		Update2DPhysics(ts);
		UpdatePhysics(ts);

		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			RenderSceneEntities();
		}
	}

	void Scene::OnUpdateRuntime(Timestep ts, EditorCamera& camera)
	{
		UpdateScripts(ts);
		Update2DPhysics(ts);
		UpdatePhysics(ts);
		Renderer2D::BeginScene(camera);
		RenderSceneEntities();
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		RenderSceneEntities();
	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		Update2DPhysics(ts);
		UpdatePhysics(ts);
		Renderer2D::BeginScene(camera);
		RenderSceneEntities();
	}


	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		//resize our non fixed aspect ratio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}


	Entity Scene::GetEntity(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();

		for (auto e : view)
		{
			Entity entity = { e, this };

			auto& idc = entity.GetComponent<IDComponent>();

			if (idc.ID == uuid)
				return entity;
		}

		return Entity();
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto e : view)
		{
			const auto& camera = view.get<CameraComponent>(e);
			if (camera.Primary)
				return Entity{e, this};
		}
		return {};
	}

	bool Scene::Valid(Entity entity) const
	{
		return m_Registry.valid(entity.GetHandle());
	}

	void Scene::RemoveLinkPoints(UUID uuid)
	{
		auto range = GetLinkPoints(uuid);
		std::vector<LinkPointMapIterator> toBeDeleted;
		for (auto it = range.first; it != range.second; it++)
			toBeDeleted.push_back(it);

		for (auto it : toBeDeleted)
			RemoveLinkPoint(it);
	}

	void Scene::OnPhysics2DStart()
	{
		m_PhysicsWorld = new b2World({ 0.f, 0.f });
		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = NativeRigidbody2DTypeToBox2D(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, 
					b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
	}

	void Scene::Update2DPhysics(Timestep ts)
	{
		m_PhysicsWorld->SetGravity({ 0.0f, -9.81 });
		m_PhysicsWorld->Step(ts, 6, 2);

		//retrieve transfrom from box2d
		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
			//here is where we could get the UUID of the entity and use a UUID to body map to get our body
			b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x; //this is where we get our updated x and y from box2d
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
		}
	}

	void Scene::OnPhysicsStart()
	{
		m_ConstrainedBodySystem = CreateRef<Enyoo::RigidBodySystem>(); 

		auto view = GetAllEntitiesWith<RigidBodyComponent, LinkPointsComponent>();
		Scope<DynamicSystemAssembler> SystemAssembler = CreateScope<DynamicSystemAssembler>(this, view);
		SystemAssembler->GenerateRigidBodies();
		SystemAssembler->GenerateConstraints();
		SystemAssembler->GenerateForceObjects();
	
		m_ConstrainedBodySystem->Initialize();
	}

	void Scene::OnPhysicsStop()
	{
	}

	void Scene::UpdatePhysics(Timestep ts)
	{
		m_ConstrainedBodySystem->Step(m_PhysicsDeltaT, m_PhysicsSteps);

		auto view = m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rbc = entity.GetComponent<RigidBodyComponent>();
			Ref<Enyoo::RigidBody> body = rbc.RuntimeBody;

			transform.Translation.x = body->Position.x;
			transform.Translation.y = body->Position.y;
			transform.Rotation.z = body->Theta;
		}
	}

	void Scene::UpdateScripts(Timestep ts)
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
				nsc.Instance->OnUpdate(ts);
		});
	}

	void Scene::RenderSceneEntities()
	{
		//Draw sprites
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
			}
		}

		//Draw circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, static_cast<int>(entity));
			}
		}

		Renderer2D::EndScene();
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<LinkPointsComponent>(Entity entity, LinkPointsComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
		
	}

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<ForceGeneratorComponent>(Entity entity, ForceGeneratorComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<ConstraintComponent>(Entity entity, ConstraintComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{

	}

	template<typename T>
	void Scene::OnComponentRemoved(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentRemoved<IDComponent>(Entity entity, IDComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<CameraComponent>(Entity entity, CameraComponent& component)
	{
		
	}

	template<>
	void Scene::OnComponentRemoved<LinkPointsComponent>(Entity entity, LinkPointsComponent& component)
	{
		RemoveLinkPoints(entity.GetUUID());
	}

	template<>
	void Scene::OnComponentRemoved<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<TagComponent>(Entity entity, TagComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<ForceGeneratorComponent>(Entity entity, ForceGeneratorComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<ConstraintComponent>(Entity entity, ConstraintComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentRemoved<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{

	}
}
