#pragma once

#include "Hazel/Math/Matrix.h"

#include <vector>

namespace Enyoo
{
	class ConjugateGradientMethod
	{
		using Matrix = Hazel::Math::Matrix;
		using Vector = Hazel::Math::Matrix;

	public:
		ConjugateGradientMethod();

		bool Solve(Matrix& leftMatrix, Vector& rightVector, Vector* resultVector);

		void SetTolerance(double tolerance) { m_Tolerance = tolerance; }
		void SetMaxIterations(size_t iterations) { m_MaxIterations = iterations; }
	private:
		size_t m_MaxIterations;
		double m_Tolerance;
	};
}
