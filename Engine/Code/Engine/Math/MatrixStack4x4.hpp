#pragma once
#include "Engine/Math/Matrix4x4.hpp"
#include <vector>

class MatrixStack4x4
{
public:
	MatrixStack4x4() { matrices.push_back(Matrix4x4::IDENTITY); };

	inline bool IsEmpty() { return (matrices.size() == 1); };
	inline Matrix4x4 GetTop() { return matrices.back(); };
	inline void PushWithoutMultiply(const Matrix4x4& transform)	{ matrices.push_back(transform); };
	inline unsigned int GetCount() const { return matrices.size(); };
	void Push(const Matrix4x4& transform);
	void Pop();


	std::vector<Matrix4x4> matrices;
};

struct ScopeCheckMatrixCount
{
	ScopeCheckMatrixCount(const MatrixStack4x4& stackToMonitor);
	~ScopeCheckMatrixCount();

	unsigned int m_currentMatrixCount;
	const MatrixStack4x4* const m_monitoredStack;
};

#define COMBINE1(X,Y) X##Y  // helper macro
#define COMBINE(X,Y) COMBINE1(X,Y)
#define ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(STACK) ScopeCheckMatrixCount COMBINE(matrixMonitor, __LINE__)(STACK);