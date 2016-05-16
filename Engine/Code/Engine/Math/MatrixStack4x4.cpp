#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

void MatrixStack4x4::Push(const Matrix4x4& transform)
{
	Matrix4x4 top = GetTop();
	Matrix4x4 newTop = Matrix4x4::IDENTITY;
	Matrix4x4::MatrixMultiply(&newTop, &transform, &top);
	matrices.push_back(newTop);
}

void MatrixStack4x4::Pop()
{
	if (!IsEmpty())
	{
		matrices.pop_back();
	}
}

ScopeCheckMatrixCount::ScopeCheckMatrixCount(const MatrixStack4x4& stackToMonitor)
	: m_monitoredStack(&stackToMonitor)
	, m_currentMatrixCount(stackToMonitor.GetCount())
{

}

ScopeCheckMatrixCount::~ScopeCheckMatrixCount()
{
	ASSERT_OR_DIE(m_currentMatrixCount == m_monitoredStack->GetCount(), Stringf("Matrix Stack left scope without the same number of matrices as entry. Began with : %i, Ended with %i.", m_currentMatrixCount, m_monitoredStack->GetCount()));
}
