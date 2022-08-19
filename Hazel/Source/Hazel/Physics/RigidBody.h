#pragma once

namespace Enyoo
{
	struct RigidBody
	{
		RigidBody() = default;

		double CalculateEnergy() const;

		size_t Index;
		glm::dvec2 Position;
		glm::dvec2 Velocity;
		double Theta;
		
		double AngularVelocity;
		double Mass;
		double MomentInertia;

		bool IgnoreGravity;
	};
}