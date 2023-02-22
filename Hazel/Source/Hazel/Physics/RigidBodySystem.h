#pragma once

#include "Hazel/Math/Matrix.h"

#include "SystemState.h"
#include "KDTree.h"
#include "RigidBody.h"
#include "GravitationalAccelerator.h"
#include "SpringForce.h"
#include "EulersMethodIntegrator.h"
#include "RungeKutta4thIntegrator.h"
#include "ConjugateGradientMethod.h"
#include "FixedPositionConstraint.h"
#include "LinkConstraint.h"

#include <vector>

namespace Enyoo
{
	static Hazel::Ref<std::vector<BodyPoint>> s_RigidBodyPoints;
	static Hazel::Ref<KDTree<BodyPoint>> s_RigidBodyKDTree;
	

	class RigidBodySystem
	{
		using Matrix = Hazel::Math::Matrix;
		using Vector = Hazel::Math::Matrix;
		using RigidBodyPtr = Hazel::Ref<RigidBody>;
		using ConstraintPtr = Hazel::Ref<Constraint>;
		using ForceGeneratorPtr = Hazel::Ref<ForceGenerator>;
		using PointerPair = std::pair<Constraint*, RigidBody*>;
		using IndexMap = std::unordered_map<PointerPair, size_t, Utilities::HashPointerFn>;

	public:

		//TODO: need a proper constructor 
		RigidBodySystem() = default;

		void Initialize();

		void Step(double dt, uint32_t steps = 1);

		void AddRigidBody(const RigidBodyPtr& body);
		void RemoveRigidBody(const RigidBodyPtr& body);
		void AddForceGen(const ForceGeneratorPtr& forceGen);
		void RemoveForceGen(const ForceGeneratorPtr& forceGen);
		void AddConstraint(const ConstraintPtr& constraint);
		void RemoveConstraint(const ConstraintPtr& constraint);

		constexpr size_t GetRigidBodyCount() const { return m_RigidBodies.size(); }
		size_t GetForceGenCount() const { return m_ForceGenerators.size(); }
		size_t GetConstraintCount() const { return m_Constraints.size(); }
		size_t GetTotalConstraintCount() const;
		std::vector<RigidBodyPtr> GetAllRigidBodies() const { return m_RigidBodies; }

		double GetTotalSystemEnergy() const;
		
		static std::vector<size_t> NnRadiusIndexSearch(RigidBody* body, double radius);
		static size_t TreeIndexToBodyIndex(size_t index);

	private:
		void PopulateSystemState();
		void PopulateMassMatrices(Matrix& Mass, Matrix& massInverse);
		void UpdateForces();
		void ResolveConstraints();
	private:
		SystemState m_State;
		
		std::vector<RigidBodyPtr> m_RigidBodies;
		std::vector<ForceGeneratorPtr> m_ForceGenerators;
		std::vector<ConstraintPtr> m_Constraints;

		RungeKutta4thIntegrator m_TimeIntegrator;
		ConjugateGradientMethod m_LinearEquationSolver;

		struct Matrices
		{
			Matrix SparseJacobian;
			Matrix SparseJacobianDot;
			Matrix Mass, W; // inverse of mass matrix 
			Vector C;
			Vector ks, kd;
			Vector Q;  // applied force global vector
			Vector qdot;
			Vector Qhat;

			Vector lambda;

			Matrix JdotQdot;
			Matrix WQ;
			Matrix JWQ;
		};


		Matrices m_Matrices;
		IndexMap m_ConstaintBodyIndex;

	};
}


/*
* TODO:
* 
* 1. Fix fixed bodies not actually being fixed in place
* 2. Correct moment of intertia for each body shape
* 3. Rolling Constraint
* 4. Speed up matrix multiplication
* 5. Add UI for step count
* 6. Support Removing and adding bodies during runtime
*/
