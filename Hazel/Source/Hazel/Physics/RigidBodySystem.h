#pragma once

#include "SystemState.h"
#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "ForceGenerator.h"
#include "Constraint.h"
#include "ODEIntegrator.h"
#include "ConjugateGradientMethod.h"

#include <vector>

namespace Enyoo
{
	class RigidBodySystem
	{
		using Matrix = Hazel::Math::Matrix;
		using Vector = Hazel::Math::Matrix;

	public:
		RigidBodySystem() = default;

		void Init(/*later we can specify the integrator*/);

		void Step(double dt, int steps = 1);

		void AddRigidBody(RigidBody* body);
		void AddForceGen(ForceGenerator* forceGen);
		void AddConstraint(Constraint* constraint);
		void RemoveRigidBody(RigidBody* body);
		void RemoveForceGen(ForceGenerator* forceGen);
		void RemoveConstraint(Constraint* constraint);

		size_t GetRigidBodyCount() const { return m_RigidBodies.size(); }
		size_t GetForceGenCount() const { return m_ForceGenerators.size(); }
		size_t GetConstraintCount() const { return m_Constraints.size(); }
		size_t GetTotalConstraintCount() const;
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
		ConjugateGradientMethod m_LinearEquationSolver;

		struct Matrices
		{
			struct BlockMatrix
			{
				size_t i; //coords into the block-sparse jacobian
				size_t j;
				size_t rows;
				size_t columns;
				Matrix BlockJacobian;
			};
			std::vector<BlockMatrix> SparseJacobian;
			std::vector<BlockMatrix> JacobianDot;
			Matrix Jc_Transpose;
			Matrix Mass, W; //inverse of mass matrix 
			Vector C;
			Vector C_ks, C_kd;
			Vector Q;
			Vector qdot;

			Vector Lambda;


		} m_MatricesData;
	};
}
