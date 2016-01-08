#pragma once

class Vector2Int
{
public:
	Vector2Int();
	Vector2Int(int initialX, int initialY);
	Vector2Int(const Vector2Int& other);
	void SetXY(int newX, int newY);

	Vector2Int& operator+=(const Vector2Int& rhs);
	Vector2Int& operator-=(const Vector2Int& rhs);
	Vector2Int& operator*=(const int& scalarConstant);

	static const Vector2Int ZERO;
	static const Vector2Int ONE;
	static const Vector2Int UNIT_X;
	static const Vector2Int UNIT_Y;

private:

public:
	int x;
	int y;
};

inline Vector2Int operator+(Vector2Int lhs, const Vector2Int& rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector2Int operator-(Vector2Int lhs, const Vector2Int& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector2Int operator*(Vector2Int lhs, const int& scalarConstant)
{
	lhs *= scalarConstant;
	return lhs;
}

inline bool operator<(Vector2Int lhs, const Vector2Int& rhs)
{
	if (lhs.y < rhs.y) return true;
	if (rhs.y < lhs.y) return false;
	return lhs.x < rhs.x;
}

inline bool operator==(const Vector2Int& lhs, const Vector2Int& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

inline bool operator!=(const Vector2Int& lhs, const Vector2Int& rhs)
{
	return !(lhs == rhs);
}