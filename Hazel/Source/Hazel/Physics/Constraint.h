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
		//Matrix J1;
		//Matrix J2;
		//Matrix J1dot;
		//Matrix J2dot;
		std::vector<Matrix> J;
		std::vector<Matrix> Jdot;
		//vbias
		//limits
		Vector ks;
		Vector kd;

	};

	class Constraint
	{
	public:
		Constraint(uint32_t ConstraintCount, uint32_t BodyCount);
		virtual ~Constraint() {}

		virtual void Calculate(ConstraintOutput& output, SystemState* state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		size_t GetConstraintCount() const { return m_ConstraintCount; }
		size_t GetBodyCount() const { return m_BodyCount; }
		size_t GetIndex() const { return m_Index; }
		RigidBody* GetBody(size_t index) { return m_Bodies[index]; }
	public: //TODO: do something about this maybe
		Matrix ConstraintForceX;
		Matrix ConstraintForceY;
		Matrix ConstraintTorque;
	protected:
		RigidBody** m_Bodies; //and this
		size_t m_Index;
		uint32_t m_BodyCount;
		uint32_t m_ConstraintCount;
	};
}
