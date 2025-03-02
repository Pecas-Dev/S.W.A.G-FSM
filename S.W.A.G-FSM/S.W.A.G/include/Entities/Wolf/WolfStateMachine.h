#pragma once

#include <Utility/SimulationConfig.h>
#include <Utility/StateMachine.h>
#include <Utility/ValueConfig.h>
#include <Entities/Entity.h>

#include <raylib.h>


class World;
class SheepStateMachine;


class WolfStateMachine : public Entity, public StateMachine
{
public:
	enum class WolfState
	{
		Sleeping,
		Roaming,
		Hunting,
		Eating,
		ReturnToDen
	};

	WolfStateMachine(World* world, Vector2 position, float scaleFactor = 1.0f);
	~WolfStateMachine();

	void Initialize();

	void Sense(float deltaTime) override;
	void Decide(float deltaTime) override;
	void Act(float deltaTime) override;
	void Draw() override;


	void DrawStatusBars(Vector2 barPosition, float staminaValue);
	void SetPosition(Vector2 newPosition);
	void DrawHuntingLine();
	void FindNewTarget();


	void SetTargetSheep(SheepStateMachine* sheep) { targetSheep = sheep; }
	void SetCurrentState(WolfState state) { currentWolfState = state; }
	void SetStamina(float value) { stamina = value; }
	void SetHunger(float value) { hunger = value; }

	float GetHungerThreshold() const { return hungerThreshold; }
	float GetStamina() const { return stamina; }
	float GetHunger() const { return hunger; }

	Vector2 GetPosition() const { return position; }

	SheepStateMachine* GetTargetSheep() const { return targetSheep; }
	WolfState GetCurrentState() const { return currentWolfState; }
	World* GetWorld() const { return world; }


	bool IsTired() const { return stamina < tiredThreshold; }
	bool IsAlive() const { return isAlive; }

	void Die() { isAlive = false; }

private:
	SheepStateMachine* targetSheep;
	WolfState currentWolfState;
	World* world;


	Vector2 position;


	Texture2D wolfTexture;


	bool isAlive;

	float hunger;
	float stamina;
	float scaleFactor;
	float targetingTimer;

	const float tiredThreshold = RuntimeConfig::WolfStaminaMax() * 0.2f;
	const float hungerThreshold = RuntimeConfig::WolfHungerThreshold();

	static constexpr float targetAcquisitionDelay = 0.5f;
	static constexpr float cellSize = ValueConfig::World::CellSize;
	static constexpr float staminaRecoveryRate = ValueConfig::Wolf::StaminaRecoveryRate;
};