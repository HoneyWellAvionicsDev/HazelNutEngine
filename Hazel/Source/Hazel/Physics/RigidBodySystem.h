#pragma once

#include "SystemState.h"
#include "RigidBody.h"
#include "ForceGenerator.h"
#include "ODEIntegrator.h"

#include <vector>

namespace Enyoo
{
	class RigidBodySystem
	{
	public:
		RigidBodySystem() = default;

		void Init(/*later we can specify the integrator*/);

		void Step(double dt, int steps = 1);

		void AddRigidBody(RigidBody* body);
		void RemoveRigidBody(RigidBody* body);
		void AddForceGen(ForceGenerator* forceGen);

		uint32_t GetRigidBodyCount() const { return (uint32_t)m_RigidBodies.size(); }
		uint32_t GetForceGenCount() const { return (uint32_t)m_ForceGenerators.size(); }
	private:
		void PopulateSystemState();
		void UpdateForces();
	private:
		SystemState m_State;

		std::vector<RigidBody*> m_RigidBodies;
		std::vector<ForceGenerator*> m_ForceGenerators;
		//std::vector<Constraint*> m_Constraints;

		ODEIntegrator m_Solver;
	};
}
