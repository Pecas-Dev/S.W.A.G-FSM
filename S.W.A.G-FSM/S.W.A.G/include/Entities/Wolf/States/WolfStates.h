#pragma once

#include <Entities/Wolf/BaseState/WolfBaseState.h>
#include <Utility/StateMachine.h>
#include <Utility/ValueConfig.h>

#include <raylib.h>


class SleepingState : public WolfBaseState
{
public:
	SleepingState(WolfStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float sleepTimer;

	static constexpr float timeToRestoreHunger = ValueConfig::Wolf::TimeToRestoreHunger;
};

class RoamingState : public WolfBaseState
{
public:
	RoamingState(WolfStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float movementTimer;

	Vector2 moveDirection;

	static constexpr float timeToChangeDirection = 1.5f;
};

class HuntingState : public WolfBaseState
{
public:
	HuntingState(WolfStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	Vector2 CalculateWolfSeparation();

	float attackTimer;
	float targetUpdateTimer;
	float stateCheckTimer;

	static constexpr float stateValidationInterval = 0.5f;
	static constexpr float separationMinDistance = 50.0f;  
	static constexpr float separationMinDistanceSquared = separationMinDistance * separationMinDistance;
	static constexpr float attackInterval = ValueConfig::Wolf::AttackInterval;
};

class WolfEatingState : public WolfBaseState
{
public:
	WolfEatingState(WolfStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float eatingTimer;

	static constexpr float timeToEat = ValueConfig::Wolf::TimeToEat;
};

class ReturnToDenState : public WolfBaseState
{
public:
	ReturnToDenState(WolfStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	static constexpr float returnSpeed = ValueConfig::Wolf::ReturnSpeed; 
	static constexpr float denProximityThreshold = 5.0f;
};