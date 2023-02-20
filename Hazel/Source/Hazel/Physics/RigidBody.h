#pragma once

namespace Enyoo
{
	struct RigidBody
	{
		RigidBody();

		double CalculateEnergy() const;
		glm::dvec2 LocalToWorld(glm::dvec2 point);
		glm::dvec2 WorldToLocal(glm::dvec2 point);

		size_t Index;
		glm::dvec2 Position;
		glm::dvec2 Velocity;
		double Theta;
		
		double AngularVelocity;
		double Mass;
		double MomentInertia;

		bool Fixed;
		bool Disable;
	};
}

