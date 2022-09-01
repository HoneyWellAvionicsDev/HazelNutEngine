#pragma once

#include "Constraint.h"

namespace Enyoo
{
	class LinkConstraint : public Constraint
	{
	public:
		LinkConstraint();
		virtual ~LinkConstraint() = default;

		virtual void Calculate(ConstraintOutput& output, SystemState* state) override;

		void SetFirstBody(RigidBody* body) { m_Bodies[0] = body; }
		void SetSecondBody(RigidBody* body) { m_Bodies[1] = body; }

		void SetFirstBodyLocal(const glm::dvec2& local) { m_FirstBodyLocal = local; }
		void SetSecondBodyLocal(const glm::dvec2& local) { m_SecondBodyLocal = local; }

	private:
		glm::dvec2 m_FirstBodyLocal;
		glm::dvec2 m_SecondBodyLocal;
		double m_Ks;
		double m_Kd;
	};
}