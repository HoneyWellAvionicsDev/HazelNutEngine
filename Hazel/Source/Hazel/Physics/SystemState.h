#pragma once

#include <glm/glm.hpp>



namespace Enyoo
{
	struct SystemState
	{
		SystemState();
		~SystemState();

		glm::vec2 LocalToWorld(glm::dvec2 point, int index);
		glm::vec2 VelocityAtPoint(glm::dvec2 point, int index);
		void ApplyForce(glm::dvec2 point, glm::dvec2 force, int index);
		void Resize(uint32_t count);
		void Destroy();

		//data
		glm::dvec2* Position;
		glm::dvec2* Velocity;
		glm::dvec2* Acceleration;
		glm::dvec2* Force;

		double* Theta;
		double* AngularVelocity;
		double* AngularAcceleration;
		double* Torque;

		uint32_t RigidBodyCount;
		uint32_t ConstraintCount;

		double dt;
	};
}
