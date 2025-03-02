#pragma once

#include <Utility/State.h>


class WolfStateMachine;


class WolfBaseState : public State
{
public:
    WolfBaseState(WolfStateMachine* stateMachine);
    virtual ~WolfBaseState() = default;

protected:
    WolfStateMachine* wolfStateMachine;
};