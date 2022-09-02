#pragma once

#include "SystemState.h"
#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "ForceGenerator.h"
#include "EulersMethodIntegrator.h"
#include "RungeKutta4thIntegrator.h"
#include "ConjugateGradientMethod.h"
#include "FixedPositionConstraint.h"
#include "LinkConstraint.h"

#include <vector>

namespace Enyoo
{
	class RigidBodySystem
	{
		using Matrix = Hazel::Math::Matrix;
		using Vector = Hazel::Math::Matrix;

	public:
		RigidBodySystem() = default;

		void Initialize(); //this should be called after all rigidbodies have been added for performance

		void Step(double dt, uint32_t steps = 1);

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

		double GetTotalSystemEnergy() const;
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

		RungeKutta4thIntegrator m_TimeIntegrator;
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
			std::vector<BlockMatrix> JacobianBlocks;
			std::vector<BlockMatrix> JacobianDotBlocks;
			
			Matrix SparseJacobian;
			Matrix SparseJacobianDot;
			Matrix Mass, W; // inverse of mass matrix 
			Vector C;
			Vector ks, kd;
			Vector Q;  // applied force global vector
			Vector qdot;
			Vector Qhat;

			Vector lambda;

			
		} 

		m_MatricesData;
	};
}
