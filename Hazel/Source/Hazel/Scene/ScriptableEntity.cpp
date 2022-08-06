#include <hzpch.h>

#include "Hazel/Core/Input.h"
#include "ScriptableEntity.h"
#include "box2d/b2_body.h"

namespace Hazel
{
	void CameraController::OnCreate()
	{
		auto& translation = GetComponent<TransformComponent>().Translation;
		translation.x = rand() % 10 - 5.f;
	}

	void CameraController::OnDestroy()
	{
	}

	void CameraController::OnUpdate(Timestep ts)
	{
		auto& translation = GetComponent<TransformComponent>().Translation;
		float speed = 5.f;

		if (Input::IsKeyPressed(HZ_KEY_A))
			translation.x -= speed * ts;
		if (Input::IsKeyPressed(HZ_KEY_D))
			translation.x += speed * ts;
		if (Input::IsKeyPressed(HZ_KEY_W))
			translation.y += speed * ts;
		if (Input::IsKeyPressed(HZ_KEY_S))
			translation.y -= speed * ts;
	}

	void Test::OnCreate()
	{
		
	}

	void Test::OnDestroy()
	{
	}

	void Test::OnUpdate(Timestep ts)
	{
		constexpr int FLOAT_MIN = 0;
		constexpr int FLOAT_MAX = 1;

		auto& sprite = GetComponent<CircleRendererComponent>();
		glm::vec4& color = sprite.Color;

		float speed = 0.01f;
		glm::vec2 velocity = { 0.0f, 0.0f };
		float v = 16.0f;

		if (Input::IsKeyPressed(HZ_KEY_W))
			velocity.y = v;
		else if (Input::IsKeyPressed(HZ_KEY_S))
			velocity.y = -v;

		if (Input::IsKeyPressed(HZ_KEY_A))
			velocity.x = -v;
		else if (Input::IsKeyPressed(HZ_KEY_D))
			velocity.x = v;

		if(Input::IsKeyPressed(HZ_KEY_SPACE))
			velocity *= 6.f;

		velocity *= speed;

		auto& rb2d = m_Entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(velocity.x, velocity.y), true);

		color.x = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
		color.y = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
		color.z = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));

		

		
	}
}
