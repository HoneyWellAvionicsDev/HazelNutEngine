#pragma once

#include <glm/glm.hpp>



namespace Enyoo
{
	struct SystemState
	{
		SystemState();
		~SystemState();

		glm::dvec2 LocalToWorld(glm::dvec2 point, size_t index);
		glm::dvec2 VelocityAtPoint(glm::dvec2 point, size_t index);
		void ApplyForce(glm::dvec2 point, glm::dvec2 force, size_t index);
		void Resize(size_t bodyCount, size_t constraintCount);
		void Destroy();

		size_t* IndexMap; 

		//data
		glm::dvec2* Position;
		glm::dvec2* Velocity;
		glm::dvec2* Acceleration; //accereration of a given body
		glm::dvec2* Force; //applied force accumulator 
		glm::dvec2* ConstraintForce; //constraint Force

		double* Theta;
		double* AngularVelocity;
		double* AngularAcceleration;
		double* Torque;
		double* ConstraintTorque;

		double* Mass;

		uint32_t RigidBodyCount;
		uint32_t ConstraintCount;

		double dt;
	};
}
