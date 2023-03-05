#pragma once

#include "MathConstants.h"
#include "Matrix.h"

#include <glm/glm.hpp>

namespace Jbonk::Math
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	float Distance(const glm::vec2 p0, const glm::vec2 p1);
}
