#include <hzpch.h>

#include "ConjugateGradientMethod.h"

#include <cmath>

namespace Enyoo
{
	ConjugateGradientMethod::ConjugateGradientMethod()
		: m_MaxIterations(300), m_Tolerance(1E-5)
	{
	}

	bool ConjugateGradientMethod::Solve(Matrix& A, Vector& b, Vector* x)
	{
		const size_t n = b.Rows();

		Vector r(n, 1);
		Vector r0;
		Vector p_k;
		p_k.Resize(n, 1);
		r0.Resize(n, 1);
		Vector x0(n, 1); // inital guess

		r = A * x0;
		r0 = r - b;
		p_k = -r0;

		for (size_t i = 0; i < m_MaxIterations; i++)
		{
			Vector Ap = A * p_k;
			const double rkrk = r0.MagnitudeSquared();
			const double alpha = rkrk / p_k.Dot(Ap);

			Matrix m_X = p_k * alpha;
			x0 = x0 + m_X;
			Vector Amx = Ap * alpha;
			r0 = r0 + Amx;

			if (std::sqrt(rkrk) < m_Tolerance)
			{
				*x = x0;
				return true;
			}

			const double rk1_mag = r0.MagnitudeSquared();
			const double beta = rk1_mag / rkrk;

			Matrix m_Y = p_k * beta;
			p_k = -r0 + m_Y;
			HZ_CORE_TRACE("Iteration: {0}", i + 1);
			x0.Print();

		}

		return false;
	}
}