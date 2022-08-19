#pragma once

#include "SystemState.h"
#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "ForceGenerator.h"
#include "Constraint.h"
#include "ODEIntegrator.h"

#include <vector>

namespace Enyoo
{
	class RigidBodySystem
	{
		using Matrix = Hazel::Math::Matrix;

	public:
		RigidBodySystem() = default;

		void Init(/*later we can specify the integrator*/);

		void Step(double dt, int steps = 1);

		void AddRigidBody(RigidBody* body);
		void RemoveRigidBody(RigidBody* body);
		void AddForceGen(ForceGenerator* forceGen);
		void AddConstraint(Constraint* constraint);

		uint32_t GetRigidBodyCount() const { return (uint32_t)m_RigidBodies.size(); }
		uint32_t GetForceGenCount() const { return (uint32_t)m_ForceGenerators.size(); }
	private:
		void PopulateSystemState();
		void PopulateMassMatrices(Matrix& Mass, Matrix& massInverse);
		void UpdateForces();
		void ResolveConstraints();
	private:
		SystemState m_State;

		std::vector<RigidBody*> m_RigidBodies;
		std::vector<ForceGenerator*> m_ForceGenerators;
		std::vector<Constraint*> m_Constraints;

		ODEIntegrator m_Solver;

		struct Matrices
		{
			//Spare Matrices: Jacobian, Jdot
			Matrix Jc_Transpose;
			Matrix Mass, W; //inverse of mass matrix 
			Matrix Constraints;
			Matrix C_ks, C_kd;
			Matrix qdot;

			Matrix Lambda;


		} m_MatricesData;
	};
}
