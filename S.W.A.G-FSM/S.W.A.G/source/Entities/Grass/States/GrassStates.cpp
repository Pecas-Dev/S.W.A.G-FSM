#include <Entities/Grass/States/GrassStates.h>
#include <Entities/Grass/GrassStateMachine.h>
#include <Utility/SimulationConfig.h>

#include <memory>


// Initializes the seeds planted state with a zero growth timer.
SeedsPlantedState::SeedsPlantedState(GrassStateMachine* stateMachine) : GrassBaseState(stateMachine), growthTimer(0.0f)
{
}

// Resets the growth timer when entering the seeds planted state.
void SeedsPlantedState::Enter()
{
	growthTimer = 0.0f;
	grassStateMachine->SetCurrentState(GrassStateMachine::GrassState::SeedsPlanted);
}

// Tracks growth progress and transitions to fully grown state when ready.
void SeedsPlantedState::Tick(float deltaTime)
{
	growthTimer += deltaTime;

	if (growthTimer >= RuntimeConfig::GrassTimeToGrow())
	{
		grassStateMachine->SwitchState(std::make_unique<FullyGrownState>(grassStateMachine));
	}
}

// Cleanup when exiting seeds planted state.
void SeedsPlantedState::Exit()
{
}

// Initializes the fully grown state with zero spread and lifetime timers.
FullyGrownState::FullyGrownState(GrassStateMachine* stateMachine) : GrassBaseState(stateMachine), spreadTimer(0.0f), lifetimeTimer(0.0f)
{
}

// Resets spread and lifetime timers when entering fully grown state.
void FullyGrownState::Enter()
{
	spreadTimer = 0.0f;
	lifetimeTimer = 0.0f;
	grassStateMachine->SetCurrentState(GrassStateMachine::GrassState::FullyGrown);
}

// Manages seed spreading and lifetime progression of fully grown grass.
void FullyGrownState::Tick(float deltaTime)
{
	spreadTimer += deltaTime;
	lifetimeTimer += deltaTime;

	if (spreadTimer >= RuntimeConfig::GrassTimeToSpread())
	{
		spreadTimer = 0.0f;
		grassStateMachine->SpreadSeeds();
	}

	if (lifetimeTimer >= RuntimeConfig::GrassLifetimeBeforeWilting())
	{
		grassStateMachine->SwitchState(std::make_unique<WiltingState>(grassStateMachine));
	}
}

// Cleanup when exiting fully grown state.
void FullyGrownState::Exit()
{
}

// Initializes the wilting state with a zero wilting timer.
WiltingState::WiltingState(GrassStateMachine* stateMachine) : GrassBaseState(stateMachine), wiltingTimer(0.0f)
{
}

// Resets the wilting timer when entering the wilting state.
void WiltingState::Enter()
{
	wiltingTimer = 0.0f;
	grassStateMachine->SetCurrentState(GrassStateMachine::GrassState::Wilting);
}

// Tracks wilting progress and triggers grass death when ready.
void WiltingState::Tick(float deltaTime)
{
	wiltingTimer += deltaTime;

	if (wiltingTimer >= RuntimeConfig::GrassTimeToWilt())
	{
		grassStateMachine->Die();
	}
}

// Cleanup when exiting wilting state.
void WiltingState::Exit()
{
}