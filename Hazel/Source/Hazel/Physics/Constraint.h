#pragma once

#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "SystemState.h"

#include <queue>

namespace Enyoo
{
	using Matrix = Jbonk::Math::Matrix;
	using Vector = Jbonk::Math::Matrix;

	struct ConstraintOutput
	{
		Vector C;
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

		virtual void Calculate(ConstraintOutput& output, SystemState& state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		std::deque<RigidBody*> GetBodies() const { return m_Bodies; }
		constexpr size_t GetConstraintCount() const { return m_ConstraintCount; }
		constexpr size_t GetBodyCount() const { return m_BodyCount; }
		constexpr size_t GetIndex() const { return m_Index; }
		RigidBody* GetBody(size_t index) { return m_Bodies[index]; }

	protected:
		std::deque<RigidBody*> m_Bodies;
		size_t m_Index;
		uint32_t m_BodyCount;
		uint32_t m_ConstraintCount;
	};
}