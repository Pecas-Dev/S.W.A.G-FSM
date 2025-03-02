#pragma once

#include <Utility/State.h>

#include <memory>


class StateMachine
{
public:
    StateMachine();
    virtual ~StateMachine();

    void SwitchState(std::unique_ptr<State> newState);

    void Update(float deltaTime);

protected:
    std::unique_ptr<State> currentState;
};
