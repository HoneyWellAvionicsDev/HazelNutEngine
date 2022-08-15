#include <hzpch.h>
#include "SystemState.h"

namespace Enyoo
{
    SystemState::SystemState()
    {
        Position = nullptr;
        Velocity = nullptr;
        Acceleration = nullptr;
        Force = nullptr;
        Theta = nullptr;
        AngularVelocity = nullptr;
        AngularAcceleration = nullptr;
        Torque = nullptr;

        RigidBodyCount = 0;
        dt = 0.0;
    }

    SystemState::~SystemState()
    {
        
    }

    glm::vec2 SystemState::LocalToWorld(glm::dvec2 point, int index)
    {
        glm::dvec2 world;
        double theta = this->Theta[index];
        world.x = point.x * glm::cos(theta) - point.y * glm::sin(theta) + Position[index].x;
        world.y = point.x * glm::sin(theta) + point.y * glm::cos(theta) - Position[index].y;
        return world;
    }

    glm::vec2 SystemState::VelocityAtPoint(glm::dvec2 point, int index)
    {
        return glm::vec2();
    }

    void SystemState::ApplyForce(glm::dvec2 point, glm::dvec2 force, int index)
    {
        glm::dvec2 w = LocalToWorld(point, index);

        this->Force[index].x += force.x;
        this->Force[index].y += force.y;
        this->Acceleration[index].x += force.x;
        this->Acceleration[index].y += force.y;

        this->Torque[index] += (w.y - this->Position[index].y) * -force.x
                            +  (w.x - this->Position[index].x) * force.y;
    }

    void SystemState::Resize(uint32_t count)
    {
        if (this->RigidBodyCount >= count)
            return;

        Destroy();

        RigidBodyCount = count;

        Position = new glm::dvec2[RigidBodyCount];
        Velocity = new glm::dvec2[RigidBodyCount];
        Acceleration = new glm::dvec2[RigidBodyCount];
        Force = new glm::dvec2[RigidBodyCount];
        Theta = new double[RigidBodyCount];
        AngularVelocity = new double[RigidBodyCount];
        AngularAcceleration = new double[RigidBodyCount];
        Torque = new double[RigidBodyCount];
    }

    void SystemState::Destroy()
    {
        delete[] Position;
        delete[] Velocity;
        delete[] Acceleration;
        delete[] Force;
        delete[] Theta;
        delete[] AngularVelocity;
        delete[] AngularAcceleration;
        delete[] Torque;

        Position = nullptr;
        Velocity = nullptr;
        Acceleration = nullptr;
        Force = nullptr;
        Theta = nullptr;
        AngularVelocity = nullptr;
        AngularAcceleration = nullptr;
        Torque = nullptr;
    }
}