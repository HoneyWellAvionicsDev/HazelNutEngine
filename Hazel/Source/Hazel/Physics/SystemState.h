#pragma once

#include <functional>
#include <unordered_map>

#include <glm/glm.hpp>
#include "Hazel/Core/UUID.h"

namespace Enyoo
{
	struct SystemState
	{
		SystemState();
		SystemState(const SystemState&);
		~SystemState();

		SystemState& operator=(const SystemState& state);

		glm::dvec2 LocalToWorld(glm::dvec2 point, size_t index);
		glm::dvec2 VelocityAtPoint(glm::dvec2 point, size_t index);
		void ApplyForce(glm::dvec2 point, glm::dvec2 force, size_t index);
		void Resize(size_t bodyCount, size_t constraintCount);
		void Destroy();

		std::shared_ptr<glm::dvec2[]> Position;
		std::shared_ptr<glm::dvec2[]> Velocity;
		std::shared_ptr<glm::dvec2[]> Acceleration; 
		std::shared_ptr<glm::dvec2[]> Force; 
		std::shared_ptr<glm::dvec2[]> ConstraintForce; 
		
		std::shared_ptr<double[]> Theta;
		std::shared_ptr<double[]> AngularVelocity;
		std::shared_ptr<double[]> AngularAcceleration;
		std::shared_ptr<double[]> Torque;
		std::shared_ptr<double[]> ConstraintTorque;

		std::shared_ptr<double[]> Mass;

		uint32_t RigidBodyCount;
		uint32_t ConstraintCount;

		double dt;
	};

	struct WorldPoint : public std::array<double, 2> //inherent from std::array because it already has the []operator implemented
	{
		static constexpr uint32_t DIM = 2;
		Hazel::UUID id;

		WorldPoint() {}
		WorldPoint(double x, double y)
		{
			(*this)[0] = x;
			(*this)[1] = y;
		}

		WorldPoint(const glm::dvec2& point)
		{
			(*this)[0] = point.x;
			(*this)[1] = point.y;
		}
	};

	struct BodyPoint
	{
		static constexpr uint32_t DIM = 2;
		size_t Index;
		glm::dvec2& Point;

		BodyPoint(glm::dvec2& point, size_t index)
			: Point(point), Index(index)
		{
		}

		BodyPoint(const BodyPoint& bodyPoint)
			: Point(bodyPoint.Point), Index(bodyPoint.Index)
		{
		}

		BodyPoint& operator=(const BodyPoint& bodyPoint) { Point = bodyPoint.Point; Index = bodyPoint.Index; return *this; }
		double& operator[](size_t pos) { return Point[pos]; }
		constexpr double& operator[](size_t pos) const { return Point[pos]; }

	};

	namespace Utilities
	{
		static glm::vec2 LocalToWorld(const glm::dvec2& local, const glm::vec3& rotation, const glm::vec3& translation)
		{
			glm::dvec2 world;
			world.x = glm::cos(rotation.z)
				* local.x - glm::sin(rotation.z)
				* local.y + translation.x;
			world.y = glm::sin(rotation.z)
				* local.x + glm::cos(rotation.z)
				* local.y + translation.y;

			return world;
		}

		template<typename T, typename U>
		static size_t HashPointerPair(T* a, U* b)
		{
			size_t seed = 0;
			size_t hash1 = reinterpret_cast<uintptr_t>(a);
			size_t hash2 = reinterpret_cast<uintptr_t>(b);
			seed ^= hash1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}

		struct HashPointerFn
		{
			template<class T, class U>
			size_t operator()(const std::pair<T*, U*>& pair) const
			{
				return HashPointerPair(pair.first, pair.second);
			}
		};
	}

	namespace PhysicalConstants
	{
		static constexpr double UNIVERSIAL_GRAVITATION = 6.6720e-11;
	}
}
