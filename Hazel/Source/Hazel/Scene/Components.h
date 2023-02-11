#pragma once

#include "Hazel/Scene/SceneCamera.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/Texture.h"

#include "Hazel/Physics/RigidBodySystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


namespace Hazel
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid)
			: ID(uuid) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag){}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.f, 0.f, 0.f, };
		glm::vec3 Rotation = { 0.f, 0.f, 0.f, }; //stored as radians
		glm::vec3 Scale = { 1.f, 1.f, 1.f, };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation){}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f };
		Ref<Texture2D> Texture;
		float TileFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct LinkPointsComponent
	{
		uint16_t Count = 0;

		LinkPointsComponent() = default;
		LinkPointsComponent(const LinkPointsComponent&) = default;
		LinkPointsComponent(uint16_t count) 
			: Count(count) {}
	};

	struct CameraComponent
	{
		SceneCamera Camera;

		bool Primary = false; //TODO: move this to scene as we will only really have one primary camera
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	class ScriptableEntity;
	enum class ScriptType : uint8_t
	{
		None = 0,
		CameraController,
		Test
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr; //to use this pointer, must #include "ScriptableEntity.h"
		
		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		ScriptType Type = ScriptType::None;

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	//Physics
	
	struct RigidBody2DComponent
	{
		enum class BodyType : uint8_t { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		void* RuntimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;
	};

	struct RigidBodyComponent
	{
		float Density = 1.0;
		bool Fixed = false;

		Ref<Enyoo::RigidBody> RuntimeBody = nullptr;

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
	};

	struct ForceGeneratorComponent
	{
		enum class GeneratorType : uint8_t { Gravity = 0, Spring, Test2 };
		GeneratorType Type = GeneratorType::Gravity;
		glm::vec2 LocalGravity = { 0.0, -9.81 };

		Ref<Enyoo::ForceGenerator> RuntimeGenerator = nullptr;

		ForceGeneratorComponent() = default;
		ForceGeneratorComponent(const ForceGeneratorComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f; //if speed goes below this threshold, then we stop simulating bounces to avoid jittering

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO: move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};


	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent,
		CircleRendererComponent, CameraComponent, LinkPointsComponent,
		NativeScriptComponent, RigidBody2DComponent, RigidBodyComponent,
		ForceGeneratorComponent,
		BoxCollider2DComponent, CircleCollider2DComponent>;
}
