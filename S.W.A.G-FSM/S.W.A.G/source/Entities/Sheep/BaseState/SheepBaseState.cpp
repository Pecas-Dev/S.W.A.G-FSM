#include <Entities/Sheep/BaseState/SheepBaseState.h>


// Initializes the Sheep base state with a reference to its state machine controller
SheepBaseState::SheepBaseState(SheepStateMachine* stateMachine) : sheepStateMachine(stateMachine)
{
}