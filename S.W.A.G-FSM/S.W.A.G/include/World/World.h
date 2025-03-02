#pragma once

#include <Entities/Grass/GrassStateMachine.h>
#include <Entities/Sheep/SheepStateMachine.h>
#include <Entities/Wolf/WolfStateMachine.h>
#include <Utility/SimulationConfig.h>
#include <Simulation/Simulation.h>

#include <raylib.h>

#include <vector>
#include <memory>


class WolfStateMachine;
class GrassStateMachine;
class Simulation;


struct SheepPoopInstance
{
	Vector2 position;

	float lifetime;

	static constexpr float maxLifetime = 10.0f;
};

struct BloodSplatterInstance
{
	Vector2 position;

	float lifetime;
	float size;

	static constexpr float maxLifetime = 5.0f;
};

class World
{
public:
	World(int width, int height, float cellSize, float viewportYOffset, float scaleFactor, Simulation* simulation);
	~World() = default;

	void Initialize(int grassCount = RuntimeConfig::WorldInitialGrassCount(), int sheepCount = RuntimeConfig::WorldInitialSheepCount(), int wolfCount = RuntimeConfig::WorldInitialWolfCount());

	void Update(float deltaTime);
	void Draw();

	void UpdateBloodSplatters(float deltaTime);
	void SpreadSeeds(const Vector2& position);
	void AddBloodSplatter(Vector2 position);
	void UpdateSheepPoop(float deltaTime);
	void AddSheepPoop(Vector2 position);
	void CreateGrass(Vector2 position);
	void CreateSheep(Vector2 position);
	void CreateWolf(Vector2 position);

	bool IsSheepPoopNearby(Vector2 position, float radius) const;
	bool IsAdjacentToGrass(Vector2 position) const;
	bool IsCellOccupied(Vector2 position) const;


	const std::vector<std::unique_ptr<WolfStateMachine>>& GetWolves() const { return wolves; }
	const std::vector<std::unique_ptr<SheepStateMachine>>& GetSheep() const { return sheeps; }
	const std::vector<std::unique_ptr<GrassStateMachine>>& GetGrasses() const { return grasses; }

	std::vector<Vector2> GetNeighboringCells(Vector2 position) const;

	Simulation* GetSimulation() const { return simulation; }

	float GetMaxY() const { return static_cast<float>(height * cellSize); }
	float GetMaxX() const { return static_cast<float>(width * cellSize); }
	float GetMinY() const { return minY; }

	float GetScaleFactor() const { return scaleFactor; }

private:
	Simulation* simulation;

	int width;
	int height;

	float minY = 0.0f;
	float cellSize;
	float scaleFactor;


	std::vector<SheepPoopInstance> sheepPoopInstances;
	std::vector<BloodSplatterInstance> bloodSplatters;

	std::vector<Vector2> pendingGrassPositions;

	std::vector<std::unique_ptr<GrassStateMachine>> grasses;
	std::vector<std::unique_ptr<WolfStateMachine>> wolves;
	std::vector<std::unique_ptr<SheepStateMachine>> sheeps;
};