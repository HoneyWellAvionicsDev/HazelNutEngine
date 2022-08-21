#pragma once

#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "SystemState.h"

#include <vector>

namespace Enyoo
{
	using Matrix = Hazel::Math::Matrix;
	using Vector = Hazel::Math::Matrix;

	struct ConstraintOutput
	{
		Vector C;
		Matrix J;
		Matrix Jdot;
		//vbias
		//limits
		Vector ks;
		Vector kd;

	};

	class Constraint
	{
	public:
		Constraint(uint32_t ConstraintCount, uint32_t BodyCount);
		virtual ~Constraint();

		virtual void Calculate(ConstraintOutput& output, SystemState* state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		size_t GetConstraintCount() const { return m_ConstraintCount; }
		size_t GetBodyCount() const { return m_Bodies.size(); }
		size_t GetIndex() const { return m_Index; }
	public: //TODO: do something about this maybe
		Matrix ConstraintForceX;
		Matrix ConstraintForceY;
		Matrix ConstraintTorque;
	protected:
		std::vector<RigidBody*> m_Bodies;
		size_t m_Index;
		size_t m_ConstraintCount;
	};
}
