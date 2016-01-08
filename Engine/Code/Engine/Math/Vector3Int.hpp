#pragma once

class Vector3Int
{
public:
	Vector3Int();
	Vector3Int(int initialX, int initialY, int initialZ);
	Vector3Int(const Vector3Int& other);
	void SetXY(int newX, int newY, int newZ);

	Vector3Int& operator+=(const Vector3Int& rhs);
	Vector3Int& operator-=(const Vector3Int& rhs);
	Vector3Int& operator*=(const int& scalarConstant);

	static const Vector3Int ZERO;
	static const Vector3Int ONE;
	static const Vector3Int UNIT_X;
	static const Vector3Int UNIT_Y;
	static const Vector3Int UNIT_Z;

public:
	int x;
	int y;
	int z;
};

inline Vector3Int operator+(Vector3Int lhs, const Vector3Int& rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector3Int operator-(Vector3Int lhs, const Vector3Int& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector3Int operator*(Vector3Int lhs, const int& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

inline bool operator==(const Vector3Int& lhs, const Vector3Int& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

inline bool operator!=(const Vector3Int& lhs, const Vector3Int& rhs)
{
	return !(lhs == rhs);
}