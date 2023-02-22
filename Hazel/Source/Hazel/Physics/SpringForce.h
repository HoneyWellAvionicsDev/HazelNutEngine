#pragma once

#include "ForceGenerator.h"

namespace Enyoo
{
	class Spring : public ForceGenerator
	{
	public:
		Spring();

		virtual void ApplyForce(SystemState& systemState) override;

		void GetEnds(glm::dvec2& firstPos, glm::dvec2& secondPos);
		double Energy() const;

		void SetRestLength(double length) { m_RestLength = length; }
		void SetFirstPosition(const glm::dvec2& position) { m_FirstBodyLocal = position; }
		void SetSecondPosition(const glm::dvec2& position) { m_SecondBodyLocal = position; }
		void SetFirstBody(RigidBody* body) { m_FirstBody = body; }
		void SetSecondBody(RigidBody* body) { m_SecondBody = body; }
		void SetSpringConstant(double k) { m_Ks = k; }
		void SetDampingValue(double value) { m_Kd = value; }
		void TorqueLock(bool lock) { m_TorqueLock = lock; }
		
	private:
		double m_RestLength;
		double m_Ks;
		double m_Kd;

		glm::dvec2 m_FirstBodyLocal;
		glm::dvec2 m_SecondBodyLocal;

		RigidBody* m_FirstBody;
		RigidBody* m_SecondBody;

		bool m_TorqueLock;
	};
}