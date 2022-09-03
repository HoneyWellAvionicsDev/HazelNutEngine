#include <hzpch.h>

#include "Hazel/Renderer/Renderer2D.h"

#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "ScriptableEntity.h"

#include <glm/glm.hpp>

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include "box2d/b2_circle_shape.h"

namespace Hazel
{
	static b2BodyType NativeRigidbody2DTypeToBox2D(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:    return b2_staticBody;
			case RigidBody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
			case RigidBody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
		}

		HZ_CORE_ASSERT(false, "Unknown body type");
		return b2_staticBody;
	}

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
		delete m_NewBodySystem;
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
				HZ_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "Component ID missing");
				entt::entity dstEnttID = enttMap.at(uuid);

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
		

		auto& srcSceneRegistry = source->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		
		for (auto it = idView.rbegin(); it != idView.rend(); it++)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(*it).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(*it).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
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

	Entity Scene::CreateBar(const glm::vec2& position, double density, const std::string& name)
	{
		Entity bar = CreateEntity(name);
		bar.AddComponent<RigidBodyComponent>();

		return bar;
		
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
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
		}

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


	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{entity, this};
		}
		return {};
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
		m_PhysicsWorld->SetGravity({ m_LocalGravity.x, m_LocalGravity.y });
		m_PhysicsWorld->Step(ts, m_VelocityIterations, m_PositionIterations);
		//HZ_CORE_TRACE("Timestep {0}", ts);

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
		m_NewBodySystem = new Enyoo::RigidBodySystem; //TODO: stop using new dumbass we have smart pointers
		//auto view = m_Registry.view<RigidBodyComponent>();
		//for (auto e : view)
		//{
		//	Entity entity = { e, this };
		//	auto& transform = entity.GetComponent<TransformComponent>();
		//	auto& rbc = entity.GetComponent<RigidBodyComponent>();
		//
		//	Enyoo::RigidBody* body = new Enyoo::RigidBody;
		//	body->Position = { transform.Translation.x, transform.Translation.y };
		//	body->Theta = transform.Rotation.z;
		//	body->Velocity = glm::dvec2{ 0.0 };
		//	body->AngularVelocity = 0.0;
		//	body->Mass = 3.0;
		//	body->MomentInertia = 1.0;
		//
		//	m_NewBodySystem->AddRigidBody(body);
		//	rbc.RuntimeBody = body;
		//}

		auto view = m_Registry.view<RigidBodyComponent>();
		auto it = view.begin();

		Enyoo::RigidBody* testbody1 = new Enyoo::RigidBody;
		testbody1->Position = { 0.0, 0.0 };
		testbody1->Theta = 0.0;
		testbody1->Velocity = glm::dvec2{ 0.0 };
		testbody1->AngularVelocity = 0.0;
		testbody1->Mass = 8.0;
		testbody1->MomentInertia = 1.0;
		auto& rbc1 = m_Registry.get<RigidBodyComponent>(*it);
		const double density1 = rbc1.Density;
		rbc1.RuntimeBody = testbody1;
		auto& asdfr = m_Registry.get<TransformComponent>(*it);
		asdfr.Scale = glm::vec3{ 5.33, 0.5, 1.0 };
		it++;
		
		Enyoo::RigidBody* testbody2 = new Enyoo::RigidBody;
		testbody2->Position = { 0.0, 0.0 };
		testbody2->Theta = 0.0;
		testbody2->Velocity = glm::dvec2{ 0.0 };
		testbody2->AngularVelocity = 0.0;
		testbody2->Mass = 10.0;
		testbody2->MomentInertia = 1.0;
		auto& rbc2 = m_Registry.get<RigidBodyComponent>(*it);
		const double density2 = rbc2.Density;
		rbc2.RuntimeBody = testbody2;
		auto& asdfr4 = m_Registry.get<TransformComponent>(*it);
		asdfr4.Scale = glm::vec3{ 5.33, 0.5, 1.0 };
		it++;

		Enyoo::RigidBody* testbody3 = new Enyoo::RigidBody;
		testbody3->Position = { 0, 1.0 };
		testbody3->Theta = 0.0;
		testbody3->Velocity = glm::dvec2{ 0.0 };
		testbody3->AngularVelocity = 0.0;
		testbody3->Mass = 4.0;
		testbody3->MomentInertia = 1.0;
		//auto& rbc3 = test3.GetComponent<RigidBodyComponent>();
		//rbc3.RuntimeBody = testbody3;
		//auto& asdf = m_Registry.get<TransformComponent>(*it);
		//asdf.Translation.x = 0;
		//asdf.Translation.y = 1;
		//asdf.Scale = glm::vec3{ 0.25 };
		//it++;
		
		Enyoo::RigidBody* testbody4 = new Enyoo::RigidBody;
		testbody4->Position = { 0.0, 0.0 };
		testbody4->Theta = 0.0;
		testbody4->Velocity = glm::dvec2{ 0.0 };
		testbody4->AngularVelocity = 0.0;
		testbody4->Mass = 10.0;
		testbody4->MomentInertia = 1.0;
		auto& rbc4 = m_Registry.get<RigidBodyComponent>(*it);
		const double density3 = rbc4.Density;
		rbc4.RuntimeBody = testbody4;
		auto& asdfr44 = m_Registry.get<TransformComponent>(*it);
		asdfr44.Scale = glm::vec3{ 5.33, 0.5, 1.0 };
		it++;
		
		Enyoo::FixedPositionConstraint* fixed1 = new Enyoo::FixedPositionConstraint;
		
		glm::dvec2 lastPosition{ 0.0, 1.0 };
		//bar1
		const double dx = 16.0 / 3.0 - lastPosition.x;
		const double dy = 1.0 - lastPosition.y;
		const double length = glm::sqrt(dx * dx + dy * dy);
		const double theta = (dy > 0) ? glm::acos(dx / length) : glm::two_pi<double>() - glm::acos(dx / length);
		
		m_NewBodySystem->AddRigidBody(testbody1);
		testbody1->Theta = theta + 1.57;
		
		glm::dvec2 world1 = testbody1->LocalToWorld({ -length / 2.0, 0.0 });
		testbody1->Position = lastPosition - world1;
		testbody1->Mass = length * density1;
		testbody1->MomentInertia = (1.0 / 12.0) * testbody1->Mass * length * length;
		
		lastPosition = testbody1->LocalToWorld({ length / 2.0, 0.0 });
		//end bar1
		
		//fix 0, 1
		glm::dvec2 local1 = testbody1->WorldToLocal({0.0, 1.0});
		m_NewBodySystem->AddConstraint(fixed1); 
		fixed1->SetBody(testbody1);
		fixed1->SetLocalPosition(local1);
		fixed1->SetWorldPosition({0.0, 1.0});
		//end fix
		
		//bar2
		const double dx2 = 16.0 / 3.0 - lastPosition.x;
		const double dy2 = 16.0 / 3.0 - lastPosition.y;
		const double length2 = glm::sqrt(dx2 * dx2 + dy2 * dy2);
		const double theta2 = (dy2 > 0) ? glm::acos(dx2 / length2) : glm::two_pi<double>() - glm::acos(dx2 / length2);
		
		m_NewBodySystem->AddRigidBody(testbody2);
		testbody2->Theta = theta2;
		
		glm::dvec2 world2 = testbody2->LocalToWorld({ -length2 / 2.0, 0.0 });
		testbody2->Position = lastPosition - world2;
		testbody2->Mass = length2 * density2;
		testbody2->MomentInertia = (1.0 / 12.0) * testbody2->Mass * length2 * length2;
		
		glm::dvec2 locallink2 = testbody1->WorldToLocal(lastPosition);
		Enyoo::LinkConstraint* link2 = new Enyoo::LinkConstraint;
		m_NewBodySystem->AddConstraint(link2);
		link2->SetFirstBody(testbody2);
		link2->SetSecondBody(testbody1);
		link2->SetFirstBodyLocal({ -length2 / 2.0, 0.0 });
		link2->SetSecondBodyLocal(locallink2);
		
		lastPosition = testbody2->LocalToWorld({ length2 / 2.0, 0.0 });
		//end bar2
		
		//bar3
		const double dx3 = 32.0 / 3.0 - lastPosition.x;
		const double dy3 = 16.0 / 3.0 - lastPosition.y;
		const double length3 = glm::sqrt(dx3 * dx3 + dy3 * dy3);
		const double theta3 = (dy3 > 0) ? glm::acos(dx3 / length3) : glm::two_pi<double>() - glm::acos(dx3 / length3);
		
		m_NewBodySystem->AddRigidBody(testbody4);
		testbody4->Theta = theta3;
		
		glm::dvec2 world3 = testbody4->LocalToWorld({ -length3 / 2.0, 0.0 });
		testbody4->Position = lastPosition - world3;
		testbody4->Mass = length3 * density3;
		testbody4->MomentInertia = (1.0 / 12.0) * testbody4->Mass * length3 * length3;
		
		glm::dvec2 locallink3 = testbody2->WorldToLocal(lastPosition);
		Enyoo::LinkConstraint* link3 = new Enyoo::LinkConstraint;
		m_NewBodySystem->AddConstraint(link3);
		link3->SetFirstBody(testbody4);
		link3->SetSecondBody(testbody2);
		link3->SetFirstBodyLocal({ -length3 / 2.0, 0.0 });
		link3->SetSecondBodyLocal(locallink3);
		
		lastPosition = testbody4->LocalToWorld({ length3 / 2.0, 0.0 });
		//end bar3
		auto view2 = m_Registry.view<ForceGeneratorComponent>();
		for (auto e : view2)
		{
			Entity entity = { e, this };
			auto& fgc = entity.GetComponent<ForceGeneratorComponent>();

			switch (fgc.Type)
			{
				case ForceGeneratorComponent::GeneratorType::Gravity:
				{
					Enyoo::GravitationalAccelerator* gravGen = new Enyoo::GravitationalAccelerator;
					gravGen->SetGravity(fgc.LocalGravity);
					m_NewBodySystem->AddForceGen(gravGen);
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
		m_NewBodySystem->Initialize();
		//m_ActiveScene->m_NewBodySystem->AddRigidBody(testbody3);
	}

	void Scene::OnPhysicsStop()
	{
	}

	void Scene::UpdatePhysics(Timestep ts)
	{
		//Timer time;
		m_NewBodySystem->Step(0.01667, 75);
		
		auto view = m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rbc = entity.GetComponent<RigidBodyComponent>();
			Enyoo::RigidBody* body = static_cast<Enyoo::RigidBody*>(rbc.RuntimeBody);
			transform.Translation.x = body->Position.x;
			transform.Translation.y = body->Position.y;
			transform.Rotation.z = body->Theta;
		}
		//HZ_CORE_TRACE("Time: {0}ms", time.ElapsedMilliseconds());
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

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}

		//Draw circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
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
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{

	}
}
