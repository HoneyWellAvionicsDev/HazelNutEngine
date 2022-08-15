#pragma once

namespace Enyoo
{
	struct RigidBody
	{
		RigidBody() = default;

		int Index;
		glm::dvec2 Position;
		glm::dvec2 Velocity;
		double Theta;
		
		double AngularVelocity;
		double Mass;
		//monment of inertia

		bool IgnoreGravity;
	};
}