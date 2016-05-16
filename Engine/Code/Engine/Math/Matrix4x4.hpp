#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include <string.h>
#include <algorithm>

class Matrix4x4
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Matrix4x4() {};
	Matrix4x4(const float* data);
	
	//STATIC FUNCTIONS//////////////////////////////////////////////////////////////////////////
	static void MatrixMakeIdentity(Matrix4x4* matrix);
	static void MatrixMakeTranslation(Matrix4x4* matrix, const Vector3& translate);
	static void MatrixMakeRotationEuler(Matrix4x4 *mat, const float yaw, const float pitch, const float roll, const Vector3 &o);
	static void MatrixMakeLookTo(Matrix4x4 *matrix, const Vector3& position, const Vector3& directionToLook, const Vector3& upVector);
	static void MatrixMakeLookAt(Matrix4x4 *mat, const Vector3& from, const Vector3& to, const Vector3& up);
	static void MatrixInvertOrthogonal(Matrix4x4* matrix);
	static void MatrixGetColumn(const Matrix4x4* matrix, int column, Vector4* out);
	static Vector4 MatrixGetColumn(const Matrix4x4* matrix, int column);
	static void MatrixSetColumn(Matrix4x4* matrix, int column, const Vector4& input);
	static void MatrixSetRow(Matrix4x4 *matrix, int row, const Vector4& input);
	static void MatrixSetRows(Matrix4x4 *mat, const Vector4 &x, const Vector4 &y, const Vector4 &z, const Vector4 &o);
	static void MatrixSetBasis(Matrix4x4 *mat, const Vector3& r, const Vector3& u, const Vector3& f, const Vector3& o);
	static void MatrixMakeProjectionOrthogonal(Matrix4x4* mat, float width, float height, float nz, float fz);
	static void MatrixGetRow(Matrix4x4 const *matrix, int row, Vector4 *out);
	static Vector4 MatrixGetRow(Matrix4x4 const *matrix, int row);
	static void MatrixMultiply(Matrix4x4 *outResult, Matrix4x4 const *leftMatrix, Matrix4x4 const *rightMatrix);
	static void MatrixInvert(Matrix4x4 *matrix);
	static void MatrixMakeRotationAroundX(Matrix4x4 *matrix, const float radians);
	static void MatrixMakeRotationAroundY(Matrix4x4 *matrix, const float radians);
	static void MatrixMakeRotationAroundZ(Matrix4x4 *matrix, const float radians);
	static void MatrixMakePerspective(Matrix4x4* mat, float width, float height, float nz, float fz);
	static void MatrixMakeScale(Matrix4x4* matrix, float scale);
	static Vector3 MatrixGetOffset(Matrix4x4 const *matrix);
	static void MatrixTranspose(Matrix4x4* matrix);
	static void MatrixSetOffset(Matrix4x4* m, const Vector3& offset);

	//MEMBER FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void SetTranslation(const Vector3& offset);
	Vector3 GetTranslation() const;
	static Vector3 MatrixGetForward(Matrix4x4* matrix);
	static void MatrixSetForward(Matrix4x4* matrix, Vector3 forward);
	static Matrix4x4 MatrixFromBasis(const Vector3& right, const Vector3& up, const Vector3& forward, const Vector3& t);
	static Matrix4x4 MatrixLerp(const Matrix4x4& a, const Matrix4x4& b, const float t);
	static void GetBasis(const Matrix4x4& a, Vector3& b1, Vector3& b2, Vector3& b3, Vector3& b4);
	void Rotate(float angle, const Vector3& Axis);
	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const Matrix4x4 IDENTITY;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	union 
	{
		float data[16];
		Vector4 column[4];
	};
};

//----------------------------------------------------------------------
inline Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs)
{
	float values[16];
	Vector4 column, row;
	for (int r = 0; r < 4; ++r) {
		Matrix4x4::MatrixGetRow(&lhs, r, &row);
		for (int c = 0; c < 4; ++c) {
			float v = Vector4::Dot(row, rhs.column[c]);
			values[(c * 4) + r] = v;
		}
	}

	return Matrix4x4(values);
}

//----------------------------------------------------------------------
inline Vector3 operator*(const Vector3& lhs, const Matrix4x4& rhs)
{
	return Vector3(
		Vector4::Dot(Vector4(lhs, 0), Matrix4x4::MatrixGetColumn(&rhs, 0)),
		Vector4::Dot(Vector4(lhs, 0), Matrix4x4::MatrixGetColumn(&rhs, 1)),
		Vector4::Dot(Vector4(lhs, 0), Matrix4x4::MatrixGetColumn(&rhs, 2))
		);
}

//----------------------------------------------------------------------
inline Vector4 operator*(const Vector4& lhs, const Matrix4x4& rhs)
{
	return Vector4(
		Vector4::Dot(lhs, Matrix4x4::MatrixGetColumn(&rhs, 0)),
		Vector4::Dot(lhs, Matrix4x4::MatrixGetColumn(&rhs, 1)),
		Vector4::Dot(lhs, Matrix4x4::MatrixGetColumn(&rhs, 2)),
		Vector4::Dot(lhs, Matrix4x4::MatrixGetColumn(&rhs, 3))
		);
}
