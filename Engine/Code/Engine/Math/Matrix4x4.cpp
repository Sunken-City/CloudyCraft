#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
//Based off of Code by Christopher Forseth
//-----------------------------------------------------------------------------------

const float identityData [16] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

const Matrix4x4 Matrix4x4::IDENTITY(identityData);

//-----------------------------------------------------------------------------------
Matrix4x4::Matrix4x4(const float* matrixData)
{
	memcpy(data, matrixData, sizeof(Matrix4x4));
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixMakeIdentity(Matrix4x4* matrix)
{
	memcpy(matrix->data, identityData, sizeof(Matrix4x4));
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixMakeTranslation(Matrix4x4* matrix, const Vector3& translate)
{
	const float data[16] = {
		1.0f, 0.0f, 0.0f, translate.x,
		0.0f, 1.0f, 0.0f, translate.y,
		0.0f, 0.0f, 1.0f, translate.z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	memcpy(matrix->data, data, sizeof(data));
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixMakePerspective(Matrix4x4* mat, float fovDegreesY, float aspect, float nz, float fz)
{
	float rads = MathUtils::DegreesToRadians(fovDegreesY);
	float size = 1.0f / tan(rads / 2.0f);

	float width = size;
	float height = size;
	if (aspect > 1.0f)
	{
		width /= aspect;
	}
	else
	{
		height *= aspect;
	}

	float q = 1.0f / (fz - nz);

	//Prof uses column major. First column is first column of the matrix.
	float const values[] = {
		width,  0.0f,       0.0f,        0,
		0.0f,       height,  0.0f,        0,
		0.0f,       0.0f,       (fz + nz) * q,  -2.0f * fz * nz * q,
		0.0f,       0.0f,       1.0f,       0.0f,
	};

	memcpy(mat->data, values, sizeof(values));
}

void Matrix4x4::MatrixMakeScale(Matrix4x4* matrix, float scale)
{
	const float data[16] = {
		scale, 0.0f, 0.0f, 0.0f,
		0.0f, scale, 0.0f, 0.0f,
		0.0f, 0.0f, scale, 0.0f,
		0.0f, 0.0f, 0.0f,  1.0f
	};
	memcpy(matrix->data, data, sizeof(data));
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixMakeProjectionOrthogonal(Matrix4x4* mat, float width, float height, float nz, float fz)
{
	float sz = 1.0f / (fz - nz);

	//Prof uses column major. First column is first column of the matrix.
	float const values[] = {
		2.0f / width,  0.0f,       0.0f,        0,
		0.0f,       2.0f / height,  0.0f,        0,
		0.0f,       0.0f,       2.0f * sz,  -(fz + nz) * sz,
		0.0f,       0.0f,       0.0f,       1.0f,
	};

	memcpy(mat->data, values, sizeof(values));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixGetRow(Matrix4x4 const *matrix, int row, Vector4 *out)
{
	*out = Vector4(matrix->data[row + 0],
		matrix->data[row + 4],
		matrix->data[row + 8],
		matrix->data[row + 12]);
}

//------------------------------------------------------------------------
Vector4 Matrix4x4::MatrixGetRow(Matrix4x4 const *matrix, int row)
{
	return Vector4(matrix->data[row + 0],
		matrix->data[row + 4],
		matrix->data[row + 8],
		matrix->data[row + 12]);
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMultiply(Matrix4x4 *outResult, Matrix4x4 const *leftMatrix, Matrix4x4 const *rightMatrix)
{
	float values[16];
	Vector4 column, row;
	for (int r = 0; r < 4; ++r) {
		MatrixGetRow(leftMatrix, r, &row);
		for (int c = 0; c < 4; ++c) {
			float v = Vector4::Dot(row, rightMatrix->column[c]);
			values[(c * 4) + r] = v;
		}
	}

	memcpy(outResult->data, values, sizeof(values));
}

//-----------------------------------------------------------------------------------
// Lifted from GLU
void Matrix4x4::MatrixInvert(Matrix4x4 *mat)
{
	float invertedData[16];
	float determinant;
	float *const m = mat->data;

	invertedData[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	invertedData[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	invertedData[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	invertedData[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	invertedData[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	invertedData[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	invertedData[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	invertedData[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	invertedData[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	invertedData[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	invertedData[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	invertedData[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	invertedData[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	invertedData[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	invertedData[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	invertedData[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	determinant = m[0] * invertedData[0] + m[1] * invertedData[4] + m[2] * invertedData[8] + m[3] * invertedData[12];

	GUARANTEE_OR_DIE(determinant != 0.0f, "Matrix not Invertable.");

	determinant = 1.0f / determinant;

	for (int i = 0; i < 16; i++) {
		m[i] = invertedData[i] * determinant;
	}
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeRotationAroundX(Matrix4x4 *matrix, const float radians)
{
	float cosr = cos(radians);
	float sinr = sin(radians);

	const float values[] = {
		1.0f,		0.0f,		0.0f,		0.0f,
		0.0f,		cosr,		sinr,	    0.0f,
		0.0f,		-sinr,		cosr,		0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
	};

	memcpy(matrix->data, values, sizeof(values));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeRotationAroundY(Matrix4x4 *matrix, const float radians)
{
	float cosr = cos(radians);
	float sinr = sin(radians);

	const float values[] = {
		cosr,		0.0f,		sinr,	   0.0f,
		0.0f,		1.0f,		0.0f,		0.0f,
		-sinr,		0.0f,		cosr,		0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
	};

	memcpy(matrix->data, values, sizeof(values));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeRotationAroundZ(Matrix4x4 *matrix, const float radians)
{
	float cosr = cos(radians);
	float sinr = sin(radians);

	const float values[] = {
		cosr,		sinr,		0.0f,		0.0f,
		-sinr,		cosr,		0.0f,		0.0f,
		0.0f,		0.0f,		1.0f,		0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
	};

	memcpy(matrix->data, values, sizeof(values));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeRotationEuler(Matrix4x4 *mat, const float yaw, const float pitch, const float roll, const Vector3 &offset)
{
	//Yaw = y
	//Pitch = x
	//Roll = z

	float sx = sin(pitch);
	float cx = cos(pitch);

	float sy = sin(yaw);
	float cy = cos(yaw);

	float sz = sin(roll);
	float cz = cos(roll);

	const float values[] = {
		cy*cz + sx*sy*sz,		cy*sz - cz*sx*sy,		cx*sy,					offset.x,
		-cx*sz,					cx*cz,					sx,						offset.y,
		-cz*sy + cy*sx*sz,		-sy*sz - cy*cz*sx,		cx*cy,					offset.z,
		0.0f,					0.0f,					0.0f,					1.0f
	};

	memcpy(mat->data, values, sizeof(values));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeLookTo(Matrix4x4* matrix, const Vector3& position, const Vector3& directionToLook, const Vector3& upVector)
{
	Vector3 up = upVector;
	Vector3 forward = directionToLook;

	up.Normalize();
	forward.Normalize();
	Vector3 right = Vector3::Cross(up, forward);
	up = Vector3::Cross(forward, right);
	up.Normalize();

	MatrixSetBasis(matrix, right, up, forward, position);
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixMakeLookAt(Matrix4x4* matrix, const Vector3& from, const Vector3& to, const Vector3& up)
{
	MatrixMakeLookTo(matrix, from, to - from, up);
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixInvertOrthogonal(Matrix4x4* matrix)
{
	Vector3 translation = MatrixGetOffset(matrix);
	MatrixTranspose(matrix);
	MatrixSetColumn(matrix, 3, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Matrix4x4 translationMatrix;
	MatrixMakeTranslation(&translationMatrix, translation);
	MatrixInvert(&translationMatrix);
	MatrixMultiply(matrix, &translationMatrix, matrix);
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixGetColumn(const Matrix4x4* matrix, int column, Vector4* out)
{
	*out = matrix->column[column];
}

//------------------------------------------------------------------------
Vector4 Matrix4x4::MatrixGetColumn(const Matrix4x4* matrix, int column)
{
	return matrix->column[column];
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixSetColumn(Matrix4x4* matrix, int column, const Vector4& input)
{
	matrix->column[column] = input;
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixSetRow(Matrix4x4 *matrix, int row, const Vector4& input)
{
	matrix->data[row + 0] = input.x;
	matrix->data[row + 4] = input.y;
	matrix->data[row + 8] = input.z;
	matrix->data[row + 12] = input.w;
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixSetRows(Matrix4x4 *mat, const Vector4 &x, const Vector4 &y, const Vector4 &z, const Vector4 &o)
{
	MatrixSetRow(mat, 0, x);
	MatrixSetRow(mat, 1, y);
	MatrixSetRow(mat, 2, z);
	MatrixSetRow(mat, 3, o);
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixSetBasis(Matrix4x4 *mat, const Vector3& r, const Vector3& u, const Vector3& f, const Vector3& o)
{
	MatrixSetRows(mat,
		Vector4(r, 0.0f),
		Vector4(u, 0.0f),
		Vector4(f, 0.0f),
		Vector4(o, 1.0f));
}

//------------------------------------------------------------------------
void Matrix4x4::MatrixSetOffset(Matrix4x4* m, const Vector3& offset)
{
	m->data[3] = offset.x;
	m->data[7] = offset.y;
	m->data[11] = offset.z;
}

//-----------------------------------------------------------------------------------
void Matrix4x4::SetTranslation(const Vector3& offset)
{
	data[3] = offset.x;
	data[7] = offset.y;
	data[11] = offset.z;
}

Vector3 Matrix4x4::GetTranslation() const
{
	return Vector3(data[3], data[7], data[11]);
}

//-----------------------------------------------------------------------------------
Vector3 Matrix4x4::MatrixGetForward(Matrix4x4* matrix)
{
	return Vector3(matrix->data[9], matrix->data[10], matrix->data[11]);
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixSetForward(Matrix4x4* matrix, Vector3 forward)
{
	matrix->data[9] = forward.x;
	matrix->data[10] = forward.y;
	matrix->data[11] = forward.z;
}

//-----------------------------------------------------------------------------------
Matrix4x4 Matrix4x4::MatrixFromBasis(const Vector3& right, const Vector3& up, const Vector3& forward, const Vector3& t)
{
	Matrix4x4 mat = Matrix4x4::IDENTITY;
	mat.data[0] = right.x;
	mat.data[4] = right.y;
	mat.data[8] = right.z;

	mat.data[1] = up.x;
	mat.data[5] = up.y;
	mat.data[9] = up.z;

	mat.data[2] = forward.x;
	mat.data[6] = forward.y;
	mat.data[10] = forward.z;

	mat.data[3] = t.x;
	mat.data[7] = t.y;
	mat.data[11] = t.z;
	return mat;
}

//-----------------------------------------------------------------------------------
Matrix4x4 Matrix4x4::MatrixLerp(const Matrix4x4& a, const Matrix4x4& b, const float time)
{
	Vector3 r0, u0, f0, t0;
	Matrix4x4::GetBasis(a, r0, u0, f0, t0);

	Vector3 r1, u1, f1, t1;
	Matrix4x4::GetBasis(b, r1, u1, f1, t1);

	Vector3 r, u, f, t;
	r = MathUtils::Lerp(time, r0, r1);
	u = MathUtils::Lerp(time, u0, u1);
	f = MathUtils::Lerp(time, f0, f1);
	t = MathUtils::Lerp(time, t0, t1);

	return MatrixFromBasis(r, u, f, t);
}

void Matrix4x4::GetBasis(const Matrix4x4& a, Vector3& b1, Vector3& b2, Vector3& b3, Vector3& b4)
{
	b1 = Vector3(a.data[0],  a.data[4],  a.data[8]);
	b2 = Vector3(a.data[1],  a.data[5],  a.data[9]);
	b3 = Vector3(a.data[2],  a.data[6],  a.data[10]);
	b4 = Vector3(a.data[3], a.data[7], a.data[11]);
}

void Matrix4x4::Rotate(float degrees, const Vector3& axis)
{
	Matrix4x4 rotation = Matrix4x4::IDENTITY;
	float cosineDegrees = MathUtils::CosDegrees(degrees);
	float sineDegrees = MathUtils::SinDegrees(degrees);
	float oneMinusCosineDegrees = (1.0f - cosineDegrees);
	rotation.data[0] = (axis.x * axis.x) * oneMinusCosineDegrees + cosineDegrees;
	rotation.data[1] = (axis.x * axis.y) * oneMinusCosineDegrees - (axis.z * sineDegrees);
	rotation.data[2] = (axis.x * axis.z) * oneMinusCosineDegrees + (axis.y * sineDegrees);
	rotation.data[3] = 0;
	rotation.data[4] = (axis.x * axis.y) * oneMinusCosineDegrees + (axis.z * sineDegrees);
	rotation.data[5] = (axis.y * axis.y) * oneMinusCosineDegrees + cosineDegrees;
	rotation.data[6] = (axis.y * axis.z) * oneMinusCosineDegrees - (axis.x * sineDegrees);
	rotation.data[7] = 0;
	rotation.data[8] = (axis.x * axis.z) * oneMinusCosineDegrees - (axis.y * sineDegrees);
	rotation.data[9] = (axis.y * axis.z) * oneMinusCosineDegrees + (axis.x * sineDegrees);
	rotation.data[10] = (axis.z * axis.z) * oneMinusCosineDegrees + cosineDegrees;
	rotation.data[11] = 0;
	rotation.data[12] = 0;
	rotation.data[13] = 0;
	rotation.data[14] = 0;
	rotation.data[15] = 1;
	Matrix4x4::MatrixMultiply(this, this, &rotation);
}

//------------------------------------------------------------------------
Vector3 Matrix4x4::MatrixGetOffset(Matrix4x4 const *matrix)
{
	return Vector3(matrix->data[3],
		matrix->data[7],
		matrix->data[11]);
}

//-----------------------------------------------------------------------------------
void Matrix4x4::MatrixTranspose(Matrix4x4* matrix)
{
	float *data = matrix->data;
	for (unsigned int y = 1; y < 4; y++)
	{
		for (unsigned int x = 0; x < y; x++)
		{
			std::swap(data[(x * 4) + y], data[(y * 4) + x]);
		}
	}
}