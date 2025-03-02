#pragma once

#include <Utility/SimulationConfig.h>
#include <Utility/StateMachine.h>
#include <Utility/ValueConfig.h>
#include <Entities/Entity.h>

#include <raylib.h>

#include <unordered_set>
#include <vector>
#include <memory>


class World;
class WolfStateMachine;


class SheepStateMachine : public Entity, public StateMachine
{
public:
	enum class SheepState
	{
		WanderingAlone,
		WanderingInGroup,
		Eating,
		Defecating,
		RunningAway,
		Reproducing
	};

	SheepStateMachine(World* world, Vector2 position, float scaleFactor = 1.0f);
	~SheepStateMachine();

	void Initialize();

	void Sense(float deltaTime) override;
	void Decide(float deltaTime) override;
	void Act(float deltaTime) override;
	void Draw() override;

	void RemoveFromGroup(SheepStateMachine* sheep);
	void AddToGroup(SheepStateMachine* sheep);
	void RequestNewSheep(Vector2 position);
	void SetPosition(Vector2 newPosition);
	void LeaveGroup();


	void SetHealth(float value) { health = std::min(value, RuntimeConfig::SheepMaxHealth()); if (health <= 0) { Die(); } }
	void SetFullness(float value) { fullness = std::min(value, RuntimeConfig::SheepMaxFullness()); }
	void SetNewSheepPositions(std::vector<Vector2>* positions) { newSheepPositions = positions; }
	void SetHunger(float value) { hunger = std::min(value, RuntimeConfig::SheepMaxHunger()); }
	void SetGroupLeader(SheepStateMachine* leader) { groupLeader = leader; }
	void SetGroupCooldownTimer(float value) { groupCooldownTimer = value; }
	void SetCurrentState(SheepState state) { currentSheepState = state; }
	void SetIsReproducing(bool value) { isReproducing = value; }

	float GetGroupCooldownTimer() const { return groupCooldownTimer; }
	float GetGroupTimer() const { return groupTimer; }
	float GetFullness() const { return fullness; }
	float GetHunger() const { return hunger; }
	float GetHealth() const { return health; }

	const std::unordered_set<SheepStateMachine*>& GetGroupMembers() const { return groupMembers; }

	Vector2 GetPosition() const { return position; }

	SheepStateMachine* GetGroupLeader() const { return groupLeader; }
	SheepState GetCurrentState() const { return currentSheepState; }
	size_t GetGroupSize() const { return groupMembers.size() + 1; }
	World* GetWorld() const { return world; }


	void UpdateGroupTimer(float deltaTime) { groupTimer += deltaTime; }
	void ResetGroupTimer() { groupTimer = 0.0f; }
	void Die() { isAlive = false; }

	bool IsInGroup() const { return groupLeader != nullptr || !groupMembers.empty(); }
	bool IsGroupLeader() const { return !groupMembers.empty(); }
	bool IsReproducing() const { return isReproducing; }
	bool IsAlive() const { return isAlive; }

private:
	void DrawStatusBars(Vector2 barPosition, float healthValue, float hungerValue);

	const WolfStateMachine* nearestWolf;
	SheepStateMachine* groupLeader;
	SheepState currentSheepState;
	World* world;


	Texture2D sheepTexture;


	std::unordered_set<SheepStateMachine*> groupMembers;

	std::vector<Vector2>* newSheepPositions;


	Vector2 position;
	Vector2 moveDirection;

	bool isAlive;
	bool isReproducing;

	float health;
	float hunger;
	float fullness;
	float groupTimer;
	float scaleFactor;
	float groupCooldownTimer = 0.0f;

	static constexpr float cellSize = ValueConfig::World::CellSize;
};