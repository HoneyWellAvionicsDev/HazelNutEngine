#include <hzpch.h>

#include "ConjugateGradientMethod.h"

#include <cmath>

namespace Enyoo
{
	ConjugateGradientMethod::ConjugateGradientMethod()
		: m_MaxIterations(30), m_Tolerance(1E-5)
	{
	}

	bool ConjugateGradientMethod::Solve(Matrix& A, Vector& b, Vector* x)
	{
		HZ_CORE_ASSERT(b.Columns() == 1);

		const size_t n = b.Rows();

		static Vector r0;
		static Vector p_k;
		p_k.Resize(n, 1);
		r0.Resize(n, 1);
		Vector x0(n, 1); // inital guess

		r0 = A * x0 - b; // r0 = b - Ax0
		p_k = -r0;       // this is the steepest gradient

		for (size_t i = 0; i < m_MaxIterations; i++)
		{
			Vector Ap = A * p_k;
			const double rkrk = r0.MagnitudeSquared(); //ri^T * ri
			const double alpha = rkrk / p_k.Dot(Ap);

			x0 += p_k * alpha;
			r0 += Ap * alpha;

			if (rkrk < m_Tolerance * m_Tolerance)
			{
				*x = x0;
				//HZ_CORE_WARN("Converged!");
				//x0.Print();
				return true;
			}

			const double rk1_mag = r0.MagnitudeSquared();
			const double beta = rk1_mag / rkrk;

			p_k = p_k * beta - r0; //d(i + 1) = r(i + 1) + Bi * di
			//x0.Print();
		}

		return false;
	}
}