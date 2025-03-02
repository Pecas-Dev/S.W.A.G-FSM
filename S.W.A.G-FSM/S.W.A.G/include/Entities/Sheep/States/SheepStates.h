#pragma once

#include <Entities/Sheep/BaseState/SheepBaseState.h>
#include <Entities/Grass/GrassStateMachine.h>
#include <Utility/SimulationConfig.h>
#include <Utility/StateMachine.h>
#include <Utility/ValueConfig.h>


#include <raylib.h>


class SheepStateMachine;


class WanderingAloneState : public SheepBaseState
{
public:
	WanderingAloneState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float movementTimer;

	Vector2 moveDirection;

	static constexpr float timeToChangeDirection = 1.5f;

	const float moveSpeed = RuntimeConfig::SheepWanderSpeed();
};

class WanderingInGroupState : public SheepBaseState
{
public:
	WanderingInGroupState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float movementTimer;

	Vector2 moveDirection;

	static constexpr float timeToChangeDirection = 2.0f;

	const float moveSpeed = RuntimeConfig::SheepWanderSpeed();
	const float groupRadius = RuntimeConfig::SheepGroupRadius();;
};

class SheepEatingState : public SheepBaseState
{
public:
	SheepEatingState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float eatingTimer;

	GrassStateMachine* targetGrass;

	static constexpr float timeToEat = ValueConfig::Sheep::TimeToEat;
};

class DefecatingState : public SheepBaseState
{
public:
	DefecatingState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float defecatingTimer;

	static constexpr float timeToDefecate = ValueConfig::Sheep::TimeToDefecate;
};

class RunningAwayState : public SheepBaseState
{
public:
	RunningAwayState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	Vector2 CalculateEdgeAvoidance(const Vector2& position);


	Vector2 randomDirectionOffset;

	float escapeDirectionTimer;

	static constexpr float directionChangeInterval = 0.7f;
	static constexpr float randomDirectionWeight = 0.4f;
	static constexpr float edgeAvoidanceWeight = 0.6f;

	const float fleeSpeed = RuntimeConfig::SheepFleeSpeed();
	const float wolfDetectionRadius = RuntimeConfig::SheepWolfDetectionRadius();
};

class ReproducingState : public SheepBaseState
{
public:
	ReproducingState(SheepStateMachine* stateMachine);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	float reproducingTimer;

	static constexpr float timeToReproduce = ValueConfig::Sheep::TimeToReproduce;
};