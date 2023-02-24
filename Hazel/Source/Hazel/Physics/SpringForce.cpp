#include "hzpch.h"
#include "SpringForce.h"


namespace Enyoo
{
	Spring::Spring()
		: m_RestLength(1.0), m_Ks(40.0), m_Kd(0.20115), m_FirstBodyLocal({0.0, 0.0}), 
		m_SecondBodyLocal({0.0, 0.0}), m_FirstBody(nullptr), m_SecondBody(nullptr), m_TorqueLock(false)
	{
	}
	
	void Spring::ApplyForce(SystemState& systemState)
	{
		if (!(m_FirstBody && m_SecondBody))
			return;

 		glm::dvec2 firstWorldPosition{ 0.0, 0.0 };
		glm::dvec2 firstPointVelocity{ 0.0, 0.0 };
		glm::dvec2 secondWorldPosition{ 0.0, 0.0 };
		glm::dvec2 secondPointVelocity{ 0.0, 0.0 };


		if (m_FirstBody->Index != -1)
		{
			firstWorldPosition = systemState.LocalToWorld(m_FirstBodyLocal, m_FirstBody->Index);
			firstPointVelocity = systemState.VelocityAtPoint(m_FirstBodyLocal, m_FirstBody->Index);
		}
		else
		{
			firstWorldPosition = m_FirstBody->LocalToWorld(m_FirstBodyLocal);
		}

		if (m_SecondBody->Index != -1)
		{
			secondWorldPosition = systemState.LocalToWorld(m_SecondBodyLocal, m_SecondBody->Index);
			secondPointVelocity = systemState.VelocityAtPoint(m_SecondBodyLocal, m_SecondBody->Index);
		}
		else
		{
			secondWorldPosition = m_SecondBody->LocalToWorld(m_SecondBodyLocal);
		}

		double dx = secondWorldPosition.x - firstWorldPosition.x;
		double dy = secondWorldPosition.y - firstWorldPosition.y;

		const double length = glm::sqrt(dx * dx + dy * dy);

		if (length != 0)
		{
			dx /= length;
			dy /= length;
		}
		else
		{
			dx = 1.0;
			dy = 0.0;
		}

		const glm::dvec2 relativeVelocity = secondPointVelocity - firstPointVelocity;
		const double v = dx * relativeVelocity.x + dy * relativeVelocity.y;
		const double x = length - m_RestLength;

		systemState.ApplyForce(
			m_FirstBodyLocal,
			{
			 dx * (x * m_Ks + v * m_Kd),
			-dy * (x * m_Ks - v * m_Kd)
			},
			m_FirstBody->Index
		);

		systemState.ApplyForce(
			m_SecondBodyLocal,
			{
			-dx * (x * m_Ks + v * m_Kd),
			 dy * (x * m_Ks - v * m_Kd)
			},
			m_SecondBody->Index
		);

		if (m_TorqueLock)
		{
			systemState.Torque[m_FirstBody->Index] = 0.0;
			systemState.Torque[m_SecondBody->Index] = 0.0;
		}
	}

	void Spring::GetEnds(glm::dvec2& firstPosition, glm::dvec2& secondPosition)
	{
		if (!(m_FirstBody && m_SecondBody))
			return;

		firstPosition = m_FirstBody->LocalToWorld(m_FirstBodyLocal);
		secondPosition = m_SecondBody->LocalToWorld(m_SecondBodyLocal);
	}

	double Spring::Energy() const
	{
		if (!(m_FirstBody || m_SecondBody))
			return 0.0;

		return 0.0;
	}
}
