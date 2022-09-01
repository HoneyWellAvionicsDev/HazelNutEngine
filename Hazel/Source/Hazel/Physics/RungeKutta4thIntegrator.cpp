#include "hzpch.h"
#include "RungeKutta4thIntegrator.h"

namespace Enyoo
{
    void RungeKutta4thIntegrator::Start(SystemState& initalState, double dt)
    {
        ODEIntegrator::Start(initalState, dt);
        SystemState t = initalState;
    }
    
    bool RungeKutta4thIntegrator::Step(SystemState& state)
    {
        return false;
    }
    
    void RungeKutta4thIntegrator::Integrate(SystemState& state)
    {
    }
}
