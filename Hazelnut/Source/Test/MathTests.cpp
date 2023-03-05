#include <hzpch.h>

#include "Hazel/Core/utest.h"

#include "Hazel/Math/Math.h"

using Jbonk::Math::Matrix;
using Vector = Matrix;

template<typename T>
concept FloatType = std::is_same_v<T, float> || std::is_same_v<T, double>;

template<FloatType T>
struct SparseMatrixTest
{
public:
	SparseMatrixTest(uint16_t rows, uint16_t columns);
	T get(uint16_t row, uint16_t column) const;
	void set(uint16_t row, uint16_t column, const T& value);
	void update_row_indices(uint16_t row);

	uint16_t _rows, _columns;

	std::vector<T> _nnz_values;
	std::vector<uint16_t> _col_index;
	std::vector<int16_t> _row_index;

};

template<FloatType T>
SparseMatrixTest<T>::SparseMatrixTest(uint16_t rows, uint16_t columns)
	: _rows(rows), _columns(columns)
{
	_row_index = std::vector<int16_t>(rows + 1, -1);
}

template<FloatType T>
void SparseMatrixTest<T>::update_row_indices(uint16_t row)
{
	if (row == _rows - 1)
		_row_index[row + 1] += 1;

	for (uint16_t i = _row_index[row]; i < _row_index.size(); i++)
		_row_index[i] += 1;
}

template<FloatType T>
void SparseMatrixTest<T>::set(uint16_t row, uint16_t column, const T& value)
{
	if (value == T())
		return;

	if (_nnz_values.empty())
	{
		_nnz_values.push_back(value);
		_col_index.push_back(column);
		_row_index[0] = 0;
		return;
	}


	size_t row_start = _row_index[row];
	size_t row_end = _row_index[row + 1] - 1;

	if (row_start == -1)
	{
		//insert to both
		_row_index[row] = 0;
		return;
	}

	if (_col_index[row_start] == column)
	{
		_nnz_values[row_start] = value;
		return;
	}

	if (_col_index[row_end] < column)
	{
		_nnz_values.insert(_nnz_values.begin() + row_end + 1, value);
		_col_index.insert(_col_index.begin() + row_end + 1, column);
		update_row_indices(row);
		return;
	}

	if (_col_index[row_start] > column)
	{
		_nnz_values.insert(_nnz_values.begin() + row_start, value);
		_col_index.insert(_col_index.begin() + row_start, column);
		update_row_indices(row);
		return;
	}

	for (size_t i = _col_index[row_start]; i < _col_index[row_end] + 1; i++)
	{
		if (i == column)
		{
			_nnz_values.insert(_nnz_values.begin() + i + 2, value);
			_col_index.insert(_col_index.begin() + i + 2, column);
			update_row_indices(row);
			return;
		}
	}
}

template<FloatType T>
T SparseMatrixTest<T>::get(uint16_t row, uint16_t column) const
{
	size_t row_start = _row_index[row];
	size_t row_end = _row_index[row + 1];
	size_t current_col = 0;

	for (size_t i = row_start; i < row_end; i++)
	{
		current_col = _col_index[i];

		if (current_col > column)
			break;

		if (current_col == column)
			return _nnz_values[i];
	}

	return T();
}

template<FloatType T>
static Vector sparse_csr_matrix_vector(const SparseMatrixTest<T>& A, const Vector& x)
{
	Vector b = Matrix(A._rows, 1);

	for (size_t row = 0; row < A._rows; row++)
	{
		T b0 = T();

		for (size_t i = A._row_index[row]; i < A._row_index[row + 1]; i++)
			b0 += A._nnz_values[i] * x[A._col_index[i]][0];

		b[row][0] = b0;
	}

	return b;
}

static Vector sparse_csr_matrix_vector_fast(const SparseMatrixTest<double>& A, const Vector& x) //this method works well for sparse matrices with dense rows
{
	Vector b = Matrix(A._rows, 1);

	for (size_t row = 0; row < A._rows; row++)
	{
		size_t row_start = A._row_index[row];
		size_t row_end = A._row_index[row + 1];
		size_t n = row_end - row_start;
		size_t residual_index = 0;
		__m256d _sum, _x_qword, _A_qword, _cbc_prod, _sumhs, _lane;
		double residual = 0.0;
		_sum = _mm256_setzero_pd();

		for (size_t i = row_start; i < row_end; i += 4)
		{
			if ((i - row_start) + 4 > n)
			{
				residual_index = i - row_start;
				break;
			}

			_x_qword = _mm256_set_pd(x[A._col_index[i + 3]][0], x[A._col_index[i + 2]][0], x[A._col_index[i + 1]][0], x[A._col_index[i]][0]);
			_A_qword = _mm256_loadu_pd(&A._nnz_values[i]);
			_cbc_prod = _mm256_mul_pd(_x_qword, _A_qword);
			_sum = _mm256_add_pd(_sum, _cbc_prod);
		}

		_sumhs = _mm256_shuffle_pd(_sum, _sum, _MM_SHUFFLE(0, 0, 1, 1));
		_sum = _mm256_add_pd(_sum, _sumhs);
		_lane = _mm256_permute2f128_pd(_sum, _sum, _MM_SHUFFLE(0, 2, 0, 1));
		_sum = _mm256_add_pd(_sum, _lane);
		b[row][0] = _mm256_cvtsd_f64(_sum);

		if (n % 4 == 0)
			continue;
		
		if (n < 4)
		{
			for (size_t i = row_start; i < row_end; i++)
				residual += A._nnz_values[i] * x[A._col_index[i]][0];
		}
		else
		{
			for (size_t i = row_start + residual_index; i < row_end; i++)
				residual += A._nnz_values[i] * x[A._col_index[i]][0];
		}

		b[row][0] += residual;
	}

	return b;
}

UTEST(SparseMatrixTest, MatrixIndex)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_index.clear();
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(20.0);
	sparseMatrix._nnz_values.push_back(30.0);
	sparseMatrix._nnz_values.push_back(40.0);
	sparseMatrix._nnz_values.push_back(50.0);
	sparseMatrix._nnz_values.push_back(60.0);
	sparseMatrix._nnz_values.push_back(70.0);
	sparseMatrix._nnz_values.push_back(80.0);

	sparseMatrix._col_index.push_back(0);
	sparseMatrix._col_index.push_back(1);
	sparseMatrix._col_index.push_back(1);
	sparseMatrix._col_index.push_back(3);
	sparseMatrix._col_index.push_back(2);
	sparseMatrix._col_index.push_back(3);
	sparseMatrix._col_index.push_back(4);
	sparseMatrix._col_index.push_back(5);

	sparseMatrix._row_index.push_back(0);
	sparseMatrix._row_index.push_back(2);
	sparseMatrix._row_index.push_back(4);
	sparseMatrix._row_index.push_back(7);
	sparseMatrix._row_index.push_back(8);

	ASSERT_EQ(sparseMatrix.get(0, 0), 10.0);
	ASSERT_EQ(sparseMatrix.get(0, 1), 20.0);
	ASSERT_EQ(sparseMatrix.get(1, 1), 30.0);
	ASSERT_EQ(sparseMatrix.get(1, 3), 40.0);
	ASSERT_EQ(sparseMatrix.get(2, 2), 50.0);
	ASSERT_EQ(sparseMatrix.get(2, 3), 60.0);
	ASSERT_EQ(sparseMatrix.get(2, 4), 70.0);
	ASSERT_EQ(sparseMatrix.get(3, 5), 80.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 4), 0.0);

	sparseMatrix.set(3, 1, 25.0);
	sparseMatrix.set(1, 0, 15.0);
	sparseMatrix.set(1, 5, 95.0);
	sparseMatrix.set(1, 2, 45.0);
	
	ASSERT_EQ(sparseMatrix._row_index[0], 0);
	ASSERT_EQ(sparseMatrix._row_index[1], 2);
	ASSERT_EQ(sparseMatrix._row_index[2], 7);
	ASSERT_EQ(sparseMatrix._row_index[3], 10);
	ASSERT_EQ(sparseMatrix._row_index[4], 12);

	ASSERT_EQ(sparseMatrix._nnz_values[0], 10.0);
	ASSERT_EQ(sparseMatrix._nnz_values[1], 20.0);
	ASSERT_EQ(sparseMatrix._nnz_values[2], 15.0);
	ASSERT_EQ(sparseMatrix._nnz_values[3], 30.0);
	ASSERT_EQ(sparseMatrix._nnz_values[4], 45.0);
	ASSERT_EQ(sparseMatrix._nnz_values[5], 40.0);
	ASSERT_EQ(sparseMatrix._nnz_values[6], 95.0);
	ASSERT_EQ(sparseMatrix._nnz_values[7], 50.0);
	ASSERT_EQ(sparseMatrix._nnz_values[8], 60.0);
	ASSERT_EQ(sparseMatrix._nnz_values[9], 70.0);
	ASSERT_EQ(sparseMatrix._nnz_values[10], 25.0);
	ASSERT_EQ(sparseMatrix._nnz_values[11], 80.0);

	ASSERT_EQ(sparseMatrix.get(0, 0), 10.0);
	ASSERT_EQ(sparseMatrix.get(0, 1), 20.0);
	ASSERT_EQ(sparseMatrix.get(1, 1), 30.0);
	ASSERT_EQ(sparseMatrix.get(1, 3), 40.0);
	ASSERT_EQ(sparseMatrix.get(2, 2), 50.0);
	ASSERT_EQ(sparseMatrix.get(2, 3), 60.0);
	ASSERT_EQ(sparseMatrix.get(2, 4), 70.0);
	ASSERT_EQ(sparseMatrix.get(3, 5), 80.0);
	ASSERT_EQ(sparseMatrix.get(1, 0), 15.0);
	ASSERT_EQ(sparseMatrix.get(1, 2), 45.0);
	ASSERT_EQ(sparseMatrix.get(1, 5), 95.0);
	ASSERT_EQ(sparseMatrix.get(3, 1), 25.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(0, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(1, 4), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 1), 0.0);
	ASSERT_EQ(sparseMatrix.get(2, 5), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 0), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 2), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 3), 0.0);
	ASSERT_EQ(sparseMatrix.get(3, 4), 0.0);
}

UTEST(SparseMatrixTest, SparseTimesVector)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_index.clear();
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(20.0);
	sparseMatrix._nnz_values.push_back(30.0);
	sparseMatrix._nnz_values.push_back(40.0);
	sparseMatrix._nnz_values.push_back(50.0);
	sparseMatrix._nnz_values.push_back(60.0);
	sparseMatrix._nnz_values.push_back(70.0);
	sparseMatrix._nnz_values.push_back(80.0);

	sparseMatrix._col_index.push_back(0);
	sparseMatrix._col_index.push_back(1);
	sparseMatrix._col_index.push_back(1);
	sparseMatrix._col_index.push_back(3);
	sparseMatrix._col_index.push_back(2);
	sparseMatrix._col_index.push_back(3);
	sparseMatrix._col_index.push_back(4);
	sparseMatrix._col_index.push_back(5);

	sparseMatrix._row_index.push_back(0);
	sparseMatrix._row_index.push_back(2);
	sparseMatrix._row_index.push_back(4);
	sparseMatrix._row_index.push_back(7);
	sparseMatrix._row_index.push_back(8);

	Vector x = Matrix(col, 1);
	x[0][0] = 2.0;
	x[1][0] = 6.0;
	x[2][0] = 9.0;
	x[3][0] = 3.0;
	x[4][0] = 1.0;
	x[5][0] = 8.0;

	Vector b = Matrix(row, 1); 
	b = sparse_csr_matrix_vector<double>(sparseMatrix, x);

	ASSERT_EQ(b[0][0], 140.0);
	ASSERT_EQ(b[1][0], 300.0);
	ASSERT_EQ(b[2][0], 700.0);
	ASSERT_EQ(b[3][0], 640.0);
}

UTEST(SparseMatrixTest, fastVectorMul)
{
	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(8, 12);
	Vector x(12, 1);
	x[0][0] = 1.0;
	x[1][0] = 2.0;
	x[2][0] = 3.0;
	x[3][0] = 4.0;
	x[4][0] = 5.0;
	x[5][0] = 6.0;
	x[6][0] = 7.0;
	x[7][0] = 8.0;
	x[8][0] = 9.0;
	x[9][0] = 10.0;
	x[10][0] = 11.0;
	x[11][0] = 12.0;

	double V[35] = { 2.0, 4.0, 1.0, 10.0, 2.0, 3.0, 4.0, 12.0, 2.0, 1.0, 1.0, 4.0, 4.0, 4.0, 2.0, 1.0, 9.0, 1.0, 10.0,
		3.0, 2.0, 9.0, 1.0, 1.0, 1.0, 12.0, 1.0, 4.0, 7.0, 4.0, 2.0, 9.0, 2.0, 3.0, 1.0 };
	size_t C[35] = { 0, 1, 2, 4, 7, 9, 10, 11, 1, 2, 6, 7, 2, 5, 8, 9, 2, 4, 9, 3, 4, 7, 2, 3, 4, 7, 10, 2, 6, 8, 10, 3, 4, 7, 11 };
	size_t R[9] = { 0, 8, 12, 16, 19, 22, 27, 31, 35 };

	for (size_t i = 0; i < 35; i++)
	{
		sparseMatrix._nnz_values.push_back(V[i]);
		sparseMatrix._col_index.push_back(C[i]);
	}

	sparseMatrix._row_index.clear();

	for (size_t i = 0; i < 9; i++)
	{
		sparseMatrix._row_index.push_back(R[i]);
	}

	Vector result = sparse_csr_matrix_vector_fast(sparseMatrix, x);

	ASSERT_EQ(result[0][0], 297.0);
	ASSERT_EQ(result[1][0], 46.0);
	ASSERT_EQ(result[2][0], 64.0);
	ASSERT_EQ(result[3][0], 132.0);
	ASSERT_EQ(result[4][0], 94.0);
	ASSERT_EQ(result[5][0], 119.0);
	ASSERT_EQ(result[6][0], 119.0);
	ASSERT_EQ(result[7][0], 82.0);
}

UTEST(Intrinsics, example)
{
	constexpr size_t row = 4;
	constexpr size_t col = 6;

	SparseMatrixTest<double> sparseMatrix = SparseMatrixTest<double>(row, col);
	sparseMatrix._row_index.clear();
	sparseMatrix._nnz_values.push_back(2.0);
	sparseMatrix._nnz_values.push_back(4.0);
	sparseMatrix._nnz_values.push_back(1.0);
	sparseMatrix._nnz_values.push_back(10.0);
	sparseMatrix._nnz_values.push_back(2.0);
	sparseMatrix._nnz_values.push_back(3.0);
	sparseMatrix._nnz_values.push_back(4.0);
	sparseMatrix._nnz_values.push_back(12.0);

	sparseMatrix._col_index.push_back(0);
	sparseMatrix._col_index.push_back(1);
	sparseMatrix._col_index.push_back(2);
	sparseMatrix._col_index.push_back(4);
	sparseMatrix._col_index.push_back(7);
	sparseMatrix._col_index.push_back(9);
	sparseMatrix._col_index.push_back(10);
	sparseMatrix._col_index.push_back(11);

	sparseMatrix._row_index.push_back(0);
	sparseMatrix._row_index.push_back(8);
	sparseMatrix._row_index.push_back(12);
	sparseMatrix._row_index.push_back(16);
	sparseMatrix._row_index.push_back(19);

	Vector vec1 = Matrix(16, 1);
	Vector vec2 = Matrix(16, 1);

	vec1[0][0] = 1.0;
	vec1[1][0] = 2.0;
	vec1[2][0] = 3.0;
	vec1[3][0] = 4.0;
	vec1[4][0] = 5.0;
	vec1[5][0] = 6.0;
	vec1[6][0] = 7.0;
	vec1[7][0] = 8.0;
	vec1[8][0] = 9.0;
	vec1[9][0] = 10.0;
	vec1[10][0] = 11.0;
	vec1[11][0] = 12.0;
	vec1[12][0] = 13.0;
	vec1[13][0] = 14.0;
	vec1[14][0] = 15.0;
	vec1[15][0] = 16.0;

	std::vector<double> values;
	values.push_back(1.0);
	values.push_back(2.0);
	values.push_back(3.0);
	values.push_back(4.0);
	values.push_back(5.0);
	values.push_back(6.0);
	values.push_back(7.0);
	values.push_back(8.0);

	Vector x = Matrix(6, 1);

	__m256d _num1, _num2, _num3, _num4, _num5, _num6, _num7, _num8;
	_num1 = _mm256_loadu_pd(&vec1[0][0]);
	_num2 = _mm256_loadu_pd(&values[0]);
	_num3 = _mm256_loadu_pd(&vec1[4][0]);
	_num4 = _mm256_loadu_pd(&values[4]);

	_num5 = _mm256_mul_pd(_num1, _num2);
	_num6 = _mm256_add_pd(_num3, _num4);
	_num7 = _mm256_fmadd_pd(_num1, _num2, _num1);

	const int shuf1  _MM_SHUFFLE(1, 1, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf2  _MM_SHUFFLE(2, 3, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf3  _MM_SHUFFLE(2, 3, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf4  _MM_SHUFFLE(2, 3, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf5  _MM_SHUFFLE(2, 3, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf6  _MM_SHUFFLE(1, 0, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf7  _MM_SHUFFLE(1, 1, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf8  _MM_SHUFFLE(1, 1, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf9  _MM_SHUFFLE(1, 0, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf10 _MM_SHUFFLE(0, 0, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf11 _MM_SHUFFLE(0, 1, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf12 _MM_SHUFFLE(1, 0, 0, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 3.0] L
	const int shuf13 _MM_SHUFFLE(0, 0, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf14 _MM_SHUFFLE(0, 0, 1, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 4.0, 3.0] W
	const int shuf15 _MM_SHUFFLE(0, 1, 1, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 4.0, 3.0] L
	const int shuf16 _MM_SHUFFLE(0, 2, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf17 _MM_SHUFFLE(2, 0, 0, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf19 _MM_SHUFFLE(0, 0, 2, 1); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 4.0] L
	const int shuf20 _MM_SHUFFLE(0, 0, 1, 2); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 2.0, 4.0, 3.0] L
	const int shuf21 _MM_SHUFFLE(0, 0, 2, 0); //[1.0, 2.0, 3.0, 4.0] -> [1.0, 1.0, 3.0, 4.0] L
	const int shuf22 _MM_SHUFFLE(0, 0, 2, 3); //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 4.0] L
	const int shuf18 _MM_SHUFFLE2(1, 1);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 3.0] L
	const int shuf23 _MM_SHUFFLE2(0, 1);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 1.0, 3.0, 3.0] L
	const int shuf24 _MM_SHUFFLE2(1, 0);      //[1.0, 2.0, 3.0, 4.0] -> [2.0, 2.0, 3.0, 3.0] L
	const int shuf25 _MM_SHUFFLE2(1, 2);      //[1.0, 2.0, 3.0, 4.0] -> [1.0, 2.0, 3.0, 3.0] L
	_num8 = _mm256_permute_pd(_num1, shuf2); 
	_num2 = _mm256_shuffle_pd(_num1, _num1, shuf2);
	__m256d n1 = _mm256_shuffle_pd(_num1, _num1, shuf19);
	__m256d n2 = _mm256_shuffle_pd(_num1, _num1, shuf23);
	__m256d n3 = _mm256_shuffle_pd(_num1, _num1, shuf14);
	__m256d n4 = _mm256_permute2f128_pd(n3, n3, 0x21);
	double testNum = _mm256_cvtsd_f64(n4);

	_mm256_storeu_pd(&vec2[0][0], _num5);
	_mm256_storeu_pd(&vec2[4][0], _num6);
	_mm256_storeu_pd(&vec2[8][0], _num7);
	_mm256_storeu_pd(&vec2[12][0], _num8);

	size_t rowIndex = 0;
	__m256d _sum, _vec, _mat_part, _cbc_prod, _sumhs, _lane;
	_sum = _mm256_setzero_pd();
	size_t n = sparseMatrix._row_index[rowIndex + 1] - sparseMatrix._row_index[rowIndex];
	for (size_t i = sparseMatrix._row_index[rowIndex]; i < n - n % 4; i += 4)
	{
		_vec = _mm256_set_pd(vec1[sparseMatrix._col_index[i + 3]][0], vec1[sparseMatrix._col_index[i + 2]][0], 
			vec1[sparseMatrix._col_index[i + 1]][0], vec1[sparseMatrix._col_index[i]][0]);

		_mat_part = _mm256_loadu_pd(&sparseMatrix._nnz_values[i]);
		_cbc_prod = _mm256_mul_pd(_vec, _mat_part);
		_sum = _mm256_add_pd(_sum, _cbc_prod);
	}
	_sumhs = _mm256_shuffle_pd(_sum, _sum, _MM_SHUFFLE(0, 0, 1, 1));
	_sum = _mm256_add_pd(_sum, _sumhs);
	_lane = _mm256_permute2f128_pd(_sum, _sum, _MM_SHUFFLE(0, 2, 0, 1));
	_sum = _mm256_add_pd(_lane, _sum);
	double sum = _mm256_cvtsd_f64(_sum);

	ASSERT_EQ(vec2[0][0], 1.0);
	ASSERT_EQ(vec2[1][0], 4.0);
	ASSERT_EQ(vec2[2][0], 9.0);
	ASSERT_EQ(vec2[3][0], 16.0);
	ASSERT_EQ(vec2[4][0], 10.0);
	ASSERT_EQ(vec2[5][0], 12.0);
	ASSERT_EQ(vec2[6][0], 14.0);
	ASSERT_EQ(vec2[7][0], 16.0);
	ASSERT_EQ(vec2[8][0], 2.0);
	ASSERT_EQ(vec2[9][0], 6.0);
	ASSERT_EQ(vec2[10][0], 12.0);
	ASSERT_EQ(vec2[11][0], 20.0);
	ASSERT_EQ(vec2[12][0], 2.0);
	ASSERT_EQ(vec2[13][0], 1.0);
	ASSERT_EQ(vec2[14][0], 4.0);
	ASSERT_EQ(vec2[15][0], 3.0);
}

