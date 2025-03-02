#pragma once

#include <Entities/Grass/BaseState/GrassBaseState.h>
#include <Utility/SimulationConfig.h>
#include <Utility/StateMachine.h>


class SeedsPlantedState : public GrassBaseState
{
public:
	SeedsPlantedState(GrassStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;


	float GetGrowthTimer() const { return growthTimer; }

private:
	float growthTimer;
};

class FullyGrownState : public GrassBaseState
{
public:
	FullyGrownState(GrassStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float spreadTimer;
	float lifetimeTimer;
};

class WiltingState : public GrassBaseState
{
public:
	WiltingState(GrassStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float wiltingTimer;
};
