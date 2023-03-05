#include <hzpch.h>

#include "ConjugateGradientMethod.h"

namespace Enyoo
{
	ConjugateGradientMethod::ConjugateGradientMethod()
		: m_MaxIterations(300), m_Tolerance(1E-10)
	{
	}

	bool ConjugateGradientMethod::Solve(Matrix& A, Vector& b, Vector& x)
	{
		JB_CORE_ASSERT(b.Columns() == 1);

		const size_t n = b.Rows();

		p_k.Resize(n, 1);
		r0.Resize(n, 1);
		Ap.Resize(n, 1);
		Apa.Resize(n, 1);
		p_ka.Resize(n, 1);

        // r0 = b - Ax0
		Matrix::Multiply(A, x0, r0);
		r0 -= b;
		Matrix::Multiply(r0, -1.0, p_k);

		for (size_t i = 0; i < m_MaxIterations; i++)
		{
			//Ap = A * p_k;
			Matrix::Multiply(A, p_k, Ap);
			const double rkrk = r0.MagnitudeSquared(); //ri^T * ri
			const double alpha = rkrk / p_k.Dot(Ap);

			Matrix::Multiply(p_k, alpha, p_ka);
			Matrix::Multiply(Ap, alpha, Apa);
			x0 += p_ka;
			r0 += Apa;

			if (r0.Magnitude() < m_Tolerance)
			{
				x = x0;
				return true; 
			}

			const double rk1_mag = r0.MagnitudeSquared();
			const double beta = rk1_mag / rkrk;

			//d(i + 1) = r(i + 1) + Bi * di
			p_k *= beta;
			p_k -= r0;
		}

		return false;
	}

	void ConjugateGradientMethod::Initialize(size_t rows)
	{
		x0.Initialize(rows, 1); // inital guess
	}
}