#pragma once

#include <Utility/State.h>


class GrassStateMachine;


class GrassBaseState : public State
{
public:
    GrassBaseState(GrassStateMachine* stateMachine);
    virtual ~GrassBaseState() = default;

protected:
    GrassStateMachine* grassStateMachine;
};
