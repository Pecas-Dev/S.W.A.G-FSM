#include <Utility/StateMachine.h>


// Initializes a new state machine with no active state.
StateMachine::StateMachine() : currentState(nullptr)
{
}

// Cleans up the current state if one exists by calling its exit method.
StateMachine::~StateMachine()
{
	if (currentState)
	{
		currentState->Exit();
	}
}

// Handles the transition between states, ensuring proper cleanup and initialization.
void StateMachine::SwitchState(std::unique_ptr<State> newState)
{
	if (currentState)
	{
		currentState->Exit();
	}

	currentState = std::move(newState);

	if (currentState)
	{
		currentState->Enter();
	}
}

// Processes the current state's tick function with the given time delta.
void StateMachine::Update(float deltaTime)
{
	if (currentState)
	{
		currentState->Tick(deltaTime);
	}
}