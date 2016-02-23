#pragma once

#include <set>
#include "Engine\Math\Vector3.hpp"
#include "Engine\Renderer\RGBA.hpp"
#include "Engine\Renderer\AABB3.hpp"

class DebugRenderer
{
public:
	//ENUMS//////////////////////////////////////////////////////////////////////////
	enum DepthTestingMode
	{
		ON,
		OFF,
		XRAY,
		NUM_MODES
	};

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	void Update(float deltaSeconds);
	void Render() const;
	void DrawDebugPoint(const Vector3& position, const RGBA& color, float duration, DepthTestingMode mode);
	void DrawDebugLine(const Vector3& start, const Vector3& end, const RGBA& color, float duration, DepthTestingMode mode);
	void DrawDebugArrow(const Vector3& start, const Vector3& forward, const RGBA& color, float duration, DepthTestingMode mode);
	void DrawDebugAABB3(const AABB3& bounds, const RGBA& strokeColor, const RGBA& fillColor, float duration, DepthTestingMode mode);
	void DrawDebugSphere(const Vector3& position, float radius, const RGBA& color, float duration, DepthTestingMode mode);

	static DebugRenderer* instance;

private:
	//---------------------------------------------------------------------------
	class Command
	{
	public:
		Command();
		virtual void Render() const;
		inline bool HasExpired() { return m_duration < 0.0f; };

		RGBA m_color;
		float m_duration;
		DebugRenderer::DepthTestingMode m_mode;
	};

	//---------------------------------------------------------------------------
	class PointCommand : public Command
	{
	public:
		PointCommand(const Vector3& position, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode);
		virtual void Render() const;

		Vector3 m_position;
	};

	//---------------------------------------------------------------------------
	class LineCommand : public Command
	{
	public:
		LineCommand(const Vector3& start, const Vector3& end, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode);
		virtual void Render() const;

		Vector3 m_start;
		Vector3 m_end;
	};

	//---------------------------------------------------------------------------
	class ArrowCommand : public Command
	{
	public:
		ArrowCommand(const Vector3& start, const Vector3& forward, const RGBA& color, float duration, DebugRenderer::DepthTestingMode mode);
		virtual void Render() const;

		Vector3 m_start;
		Vector3 m_end;
	};

	//---------------------------------------------------------------------------
	class AABB3Command : public Command
	{
	public:
		AABB3Command(const AABB3& bounds, const RGBA& strokeColor, const RGBA& fillColor, float duration, DepthTestingMode mode);
		virtual void Render() const;

		AABB3 m_bounds;
		RGBA m_fillColor;
	};

	//---------------------------------------------------------------------------
	class SphereCommand : public Command
	{
	public:
		SphereCommand(const Vector3& position, float radius, const RGBA& color, float duration, DepthTestingMode mode);
		~SphereCommand();
		virtual void Render() const;

		Vector3 m_position;
		float m_radius;
		int m_vboId;
		int m_numVerts;
	};

	static std::set<Command*> s_renderCommands;
};