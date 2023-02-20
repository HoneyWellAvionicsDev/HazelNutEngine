#pragma once

#include "ForceGenerator.h"

namespace Enyoo
{
	class LocalGravity : public ForceGenerator
	{
	public:
		LocalGravity();

		virtual void ApplyForce(SystemState& state) override;

		void SetGravity(glm::dvec2 gravity) { m_LocalGravity = gravity; }
		glm::dvec2 GetGravity() const { return m_LocalGravity; }
	private:
		glm::dvec2 m_LocalGravity;
	};

	class GravitationalAccelerator : public ForceGenerator
	{
	public:
		GravitationalAccelerator();

		virtual void ApplyForce(SystemState& state) override;

		void SetSourceBody(RigidBody* body) { m_SourceBody = body; }
		void SetInfluenceRadius(double radius) { m_InfluenceRadius = radius; }
		void SetRepulisveForce(bool force) { m_AntiForce = force; }
	private:
		RigidBody* m_SourceBody;
		uint32_t m_IterationCounter;
		double m_InfluenceRadius;
		bool m_AntiForce;
	};
}