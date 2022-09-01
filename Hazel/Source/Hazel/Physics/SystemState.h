#pragma once

#include <glm/glm.hpp>



namespace Enyoo
{
	struct SystemState
	{
		SystemState();
		SystemState(const SystemState&);
		~SystemState();

		SystemState& operator=(const SystemState& state);

		glm::dvec2 LocalToWorld(glm::dvec2 point, size_t index);
		glm::dvec2 VelocityAtPoint(glm::dvec2 point, size_t index);
		void ApplyForce(glm::dvec2 point, glm::dvec2 force, size_t index);
		void Resize(size_t bodyCount, size_t constraintCount);
		void Destroy();

		//data
		Hazel::Scope<glm::dvec2[]> Position;
		Hazel::Scope<glm::dvec2[]> Velocity;
		Hazel::Scope<glm::dvec2[]> Acceleration; //accereration of a given body
		Hazel::Scope<glm::dvec2[]> Force; //applied force accumulator 
		Hazel::Scope<glm::dvec2[]> ConstraintForce; //constraint Force

		Hazel::Scope<double[]> Theta;
		Hazel::Scope<double[]> AngularVelocity;
		Hazel::Scope<double[]> AngularAcceleration;
		Hazel::Scope<double[]> Torque;
		Hazel::Scope<double[]> ConstraintTorque;

		Hazel::Scope<double[]> Mass;

		uint32_t RigidBodyCount;
		uint32_t ConstraintCount;

		double dt;
	};
}
