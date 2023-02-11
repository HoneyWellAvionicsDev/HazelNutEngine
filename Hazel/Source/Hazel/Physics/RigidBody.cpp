#include "hzpch.h"
#include "RigidBody.h"

namespace Enyoo
{
    RigidBody::RigidBody()
        : Index(-1), Position{0.0, 0.0}, Velocity{0.0, 0.0}, Theta(0.0), Mass(0.0), AngularVelocity(0.0), MomentInertia(0.0), Fixed(false)
    {
    }

    double RigidBody::CalculateEnergy() const
    {
        const double kineticEnergy = 0.5 * Mass * (Velocity.x * Velocity.x + Velocity.y * Velocity.y);
        const double rotationalEnergy = 0.5 * MomentInertia * AngularVelocity * AngularVelocity;
        const double potentialEnergy = 9.8 * Mass * (Position.y - -100);

        return kineticEnergy + rotationalEnergy + potentialEnergy;
    }
    
    glm::dvec2 RigidBody::LocalToWorld(glm::dvec2 point)
    {
        //p(t) = R(t)p0 + x(t) where R is some rotation matrix, x is the translation vector, p0 is a point on the rigidbody
        glm::dvec2 world;
        world.x = glm::cos(Theta) * point.x - glm::sin(Theta) * point.y + Position.x;
        world.y = glm::sin(Theta) * point.x + glm::cos(Theta) * point.y + Position.y;
        return world;
    }
    
    glm::dvec2 RigidBody::WorldToLocal(glm::dvec2 point)
    {
        glm::dvec2 local;
        local.x = (point.x - Position.x) *  glm::cos(Theta) + (point.y - Position.y) * glm::sin(Theta);
        local.y = (point.x - Position.x) * -glm::sin(Theta) + (point.y - Position.y) * glm::cos(Theta);
        return local;
    }
}
