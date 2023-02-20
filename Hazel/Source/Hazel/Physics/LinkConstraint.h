#pragma once

#include "Constraint.h"

namespace Enyoo
{
	class LinkConstraint : public Constraint
	{
	public:
		LinkConstraint();
		virtual ~LinkConstraint() = default;

		virtual void Calculate(ConstraintOutput& output, SystemState& state) override;

		void SetFirstBody(RigidBody* body) { m_Bodies.push_front(body); }
		void SetSecondBody(RigidBody* body) { m_Bodies.push_back(body); }

		void SetFirstBodyLocal(const glm::dvec2& local) { m_FirstBodyLocal = local; }
		void SetSecondBodyLocal(const glm::dvec2& local) { m_SecondBodyLocal = local; }

	private:
		glm::dvec2 m_FirstBodyLocal;
		glm::dvec2 m_SecondBodyLocal;
		double m_ks;
		double m_kd;

		Matrix J1;
		Matrix J2;
		Matrix J1dot;
		Matrix J2dot;
	};
}
