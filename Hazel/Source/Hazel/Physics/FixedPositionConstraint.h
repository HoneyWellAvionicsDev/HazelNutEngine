#pragma once

#include "Constraint.h"

namespace Enyoo
{
	static constexpr int MaxConstraintCount = 3;
	static constexpr int MaxBodyCount = 2;

	class FixedPositionConstraint : public Constraint
	{
	public:
		FixedPositionConstraint();
		virtual ~FixedPositionConstraint();

		void SetBody(RigidBody* body, size_t index = 0) { m_Bodies[index] = body; }

		void SetWorldPosition(glm::dvec2 position) { m_WorldPosition = position; }
		void SetLocalPosition(glm::dvec2 position) { m_LocalPosition = position; }

		virtual void Calculate(ConstraintOutput& output, SystemState* state) override;
	private:
		glm::dvec2 m_LocalPosition;
		glm::dvec2 m_WorldPosition;
		double m_ks;
		double m_kd;

		Matrix J1;
		Matrix J1dot;
	};
}