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
	static void MatrixSetColumn(Matrix4x4* matrix, int column, const Vector4& input);
	static void MatrixSetRow(Matrix4x4 *matrix, int row, const Vector4& input);
	static void MatrixSetRows(Matrix4x4 *mat, const Vector4 &x, const Vector4 &y, const Vector4 &z, const Vector4 &o);
	static void MatrixSetBasis(Matrix4x4 *mat, const Vector3& r, const Vector3& u, const Vector3& f, const Vector3& o);
	static void MatrixMakeProjectionOrthogonal(Matrix4x4* mat, float width, float height, float nz, float fz);
	static void MatrixGetRow(Matrix4x4 const *matrix, int row, Vector4 *out);
	static void MatrixMultiply(Matrix4x4 *outResult, Matrix4x4 const *leftMatrix, Matrix4x4 const *rightMatrix);
	static void MatrixInvert(Matrix4x4 *matrix);
	static void MatrixMakeRotationAroundX(Matrix4x4 *matrix, const float radians);
	static void MatrixMakeRotationAroundY(Matrix4x4 *matrix, const float radians);
	static void MatrixMakeRotationAroundZ(Matrix4x4 *matrix, const float radians);
	static void MatrixMakePerspective(Matrix4x4* mat, float width, float height, float nz, float fz);
	static Vector3 MatrixGetOffset(Matrix4x4 const *matrix);
	static void MatrixTranspose(Matrix4x4* matrix);
	static void MatrixSetOffset(Matrix4x4* m, const Vector3& offset);

	//MEMBER FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void SetTranslation(const Vector3& offset);

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const Matrix4x4 IDENTITY;

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	union 
	{
		float data[16];
		Vector4 column[4];
	};
};