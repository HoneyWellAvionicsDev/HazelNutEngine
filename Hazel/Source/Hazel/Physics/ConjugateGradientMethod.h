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

		bool Solve(Matrix& leftMatrix, Vector& rightVector, Vector& resultVector);

		void Initialize(size_t rows);

		void SetTolerance(double tolerance) { m_Tolerance = tolerance; }
		void SetMaxIterations(size_t iterations) { m_MaxIterations = iterations; }
	private:
		size_t m_MaxIterations;
		double m_Tolerance;
		Vector Ap;
		Vector p_ka;
		Vector Apa;
		Vector r0;
		Vector p_k;
		Vector x0;
	};
}
