#include <Entities/Wolf/BaseState/WolfBaseState.h>


// Initializes the Wolf base state with a reference to its state machine controller
WolfBaseState::WolfBaseState(WolfStateMachine* stateMachine) : wolfStateMachine(stateMachine)
{
}
