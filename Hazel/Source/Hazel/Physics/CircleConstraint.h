#pragma once

#include "Constraint.h"

namespace Enyoo
{ 
	class CircleConstraint : public Constraint
	{
	public:
		CircleConstraint();
		virtual ~CircleConstraint() = default;

		virtual void Calculate(ConstraintOutput& output, SystemState& state) override;

		void SetBaseBody(RigidBody* body) { m_Bodies.push_front(body); }
		void SetRollingBody(RigidBody* body) { m_Bodies.push_back(body); }
		void SetLocal(const glm::dvec2& position) { m_LocalPosition = position; }
		void SetRadius(double radius) { m_Radius = radius; }

	private:
		glm::dvec2 m_LocalPosition;
		double m_Dx;
		double m_Dy;
		double m_Radius;

		double m_Ks;
		double m_Kd;

		Matrix J1;
		Matrix J2;
		Matrix J1dot;
		Matrix J2dot;
	};
}

