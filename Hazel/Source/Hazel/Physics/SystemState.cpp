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
        ConstraintForce = nullptr;
        Theta = nullptr;
        AngularVelocity = nullptr;
        AngularAcceleration = nullptr;
        Torque = nullptr;
        ConstraintTorque = nullptr;
        Mass = nullptr;

        RigidBodyCount = 0;
        ConstraintCount = 0;
        dt = 0.0;
    }

    SystemState::SystemState(const SystemState& state)
    {
        Resize(state.RigidBodyCount, state.ConstraintCount);

        this->dt = state.dt;

        for (uint32_t i = 0; i < state.RigidBodyCount; i++)
        {
            this->Position[i] = state.Position[i];
            this->Velocity[i] = state.Velocity[i];
            this->Acceleration[i] = state.Acceleration[i];

            this->Theta[i] = state.Theta[i];
            this->AngularVelocity[i] = state.AngularVelocity[i];
            this->AngularAcceleration[i] = state.AngularAcceleration[i];

            this->Force[i] = state.Force[i];
            this->Torque[i] = state.Torque[i];

            this->Mass[i] = state.Mass[i];
        }

        for (uint32_t i = 0; i < state.ConstraintCount; i++)
        {
            this->ConstraintForce[i] = state.ConstraintForce[i];
            this->ConstraintTorque[i] = state.ConstraintTorque[i];
        }
    }

    SystemState::~SystemState()
    {
        Destroy();
    }

    SystemState& SystemState::operator=(const SystemState& state)
    {
        Resize(state.RigidBodyCount, state.ConstraintCount);

        this->dt = state.dt;

        for (uint32_t i = 0; i < state.RigidBodyCount; i++)
        {
            this->Position[i] = state.Position[i];
            this->Velocity[i] = state.Velocity[i];
            this->Acceleration[i] = state.Acceleration[i];

            this->Theta[i] = state.Theta[i];
            this->AngularVelocity[i] = state.AngularVelocity[i];
            this->AngularAcceleration[i] = state.AngularAcceleration[i];

            this->Force[i] = state.Force[i];
            this->Torque[i] = state.Torque[i];

            this->Mass[i] = state.Mass[i];
        }

        for (uint32_t i = 0; i < state.ConstraintCount; i++)
        {
            this->ConstraintForce[i] = state.ConstraintForce[i];
            this->ConstraintTorque[i] = state.ConstraintTorque[i];
        }

        return *this;
    }

    glm::dvec2 SystemState::LocalToWorld(glm::dvec2 point, size_t index)
    {
        glm::dvec2 world;
        double theta = this->Theta[index];

        world.x = point.x * glm::cos(theta) - point.y * glm::sin(theta) + Position[index].x;
        world.y = point.x * glm::sin(theta) + point.y * glm::cos(theta) - Position[index].y;

        return world;
    }

    glm::dvec2 SystemState::VelocityAtPoint(glm::dvec2 point, size_t index)
    {
        glm::dvec2 velocity;
        glm::dvec2 world = LocalToWorld(point, index);

        velocity.x = this->Velocity[index].x - this->AngularVelocity[index] * (world.y - this->Position[index].y);
        velocity.y = this->Velocity[index].y + this->AngularVelocity[index] * (world.x - this->Position[index].x);

        return velocity;
    }

    void SystemState::ApplyForce(glm::dvec2 point, glm::dvec2 force, size_t index)
    {
        glm::dvec2 world = LocalToWorld(point, index);
        glm::dvec2 r = point - world;
        float theta = 3.1415 / 2.0f;

        this->Force[index].x += force.x;
        this->Force[index].y += force.y;

        this->Torque[index] += (world.y - this->Position[index].y) * -force.x
                            +  (world.x - this->Position[index].x) * force.y;
        //this->Torque[index] += glm::length(force) * glm::length(r) * glm::sin(theta);
    }

    void SystemState::Resize(size_t bodyCount, size_t constraintCount)
    {
        if (this->RigidBodyCount >= bodyCount && this->ConstraintCount >= constraintCount)
            return;

        Destroy();

        this->RigidBodyCount = bodyCount;
        this->ConstraintCount = constraintCount;

        this->Position            = std::make_shared<glm::dvec2[]>(RigidBodyCount);
        this->Velocity            = std::make_shared<glm::dvec2[]>(RigidBodyCount);
        this->Acceleration        = std::make_shared<glm::dvec2[]>(RigidBodyCount);
        this->Force               = std::make_shared<glm::dvec2[]>(RigidBodyCount);
        this->ConstraintForce     = std::make_shared<glm::dvec2[]>(ConstraintCount * 2);
        this->Theta               = std::make_shared<double[]>(RigidBodyCount);
        this->AngularVelocity     = std::make_shared<double[]>(RigidBodyCount);
        this->AngularAcceleration = std::make_shared<double[]>(RigidBodyCount);
        this->Torque              = std::make_shared<double[]>(RigidBodyCount);
        this->ConstraintTorque    = std::make_shared<double[]>(ConstraintCount * 2);
        this->Mass                = std::make_shared<double[]>(RigidBodyCount);
    }

    void SystemState::Destroy()
    {
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