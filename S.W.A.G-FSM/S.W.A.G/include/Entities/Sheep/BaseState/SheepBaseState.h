#include <Utility/State.h>


class SheepStateMachine;


class SheepBaseState : public State
{
public:
    SheepBaseState(SheepStateMachine* stateMachine);
    virtual ~SheepBaseState() = default;

protected:
    SheepStateMachine* sheepStateMachine;
};

