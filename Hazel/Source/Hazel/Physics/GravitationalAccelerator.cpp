#include "hzpch.h"

#include "GravitationalAccelerator.h"
#include "RigidBodySystem.h"

namespace Enyoo
{
	LocalGravity::LocalGravity()
		: m_LocalGravity{0.0, -9.81}
	{
	}

	void LocalGravity::ApplyForce(SystemState& state)
	{	
		for (size_t i = 0; i < state.RigidBodyCount; i++)
		{
			state.Force[i] += state.Mass[i] * m_LocalGravity;
		}
	}

	GravitationalAccelerator::GravitationalAccelerator()
		: m_InfluenceRadius(0.0), m_SourceBody(nullptr), m_IterationCounter(0), m_AntiForce(false)
	{
	}

	void GravitationalAccelerator::ApplyForce(SystemState& state)
	{
		//if (m_IterationCounter++ % 4 != 0)
		//	return;

		std::vector<size_t> InfluencedBodies = RigidBodySystem::NnRadiusIndexSearch(m_SourceBody, m_InfluenceRadius);
		
		for (size_t index : InfluencedBodies)
		{
			size_t bodyIndex = RigidBodySystem::TreeIndexToBodyIndex(index);
			
			if (bodyIndex == m_SourceBody->Index)
				continue;

			glm::dvec2 Fg;
			glm::dvec2 r = m_SourceBody->Position - state.Position[bodyIndex];
			double rMag = glm::sqrt(r.x * r.x + r.y * r.y);
			glm::dvec2 rHat{ r.x / rMag, r.y / rMag };
			double Force = PhysicalConstants::UNIVERSIAL_GRAVITATION * m_SourceBody->Mass * state.Mass[bodyIndex] / rMag * rMag;
			Force *= 1e8;

			if (m_AntiForce)
				Force *= -1;
		
			Fg.x = Force * rHat.x;
			Fg.y = Force * rHat.y;
			//JB_CORE_TRACE("Force x: {0}, y: {1}", Fg.x, Fg.y);
			//state.ApplyForce({0.0, 0.0}, Fg, bodyIndex);
			//state.Torque[bodyIndex] = 0.0;
			state.Force[bodyIndex] += Fg;
		}
	}
}

