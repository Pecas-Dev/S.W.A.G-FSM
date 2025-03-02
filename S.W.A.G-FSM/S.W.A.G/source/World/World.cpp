#include <Entities/Sheep/States/SheepStates.h>
#include <Entities/Grass/GrassStateMachine.h>
#include <Entities/Sheep/SheepStateMachine.h>
#include <Entities/Wolf/WolfStateMachine.h>
#include <Utility/SimulationConfig.h>
#include <Utility/ValueConfig.h>
#include <World/World.h>


// Creates a new world with specified dimensions and cell size.
World::World(int width, int height, float cellSize, float viewportYOffset, float scaleFactor, Simulation* simulation) : width(width), height(height), cellSize(cellSize* scaleFactor), minY(viewportYOffset), scaleFactor(scaleFactor), simulation(simulation)
{
}

// Initializes the world with a specified number entities.
void World::Initialize(int grassCount, int sheepCount, int wolfCount)
{
	float minimumY = GetMinY();

	// Initialize grass
	for (int i = 0; i < grassCount; i++)
	{
		Vector2 position;

		do
		{
			position.x = static_cast<float>(GetRandomValue(0, width - 1));
			position.y = static_cast<float>(GetRandomValue(0, height - 1));

			if (position.y * cellSize * scaleFactor + minY < minimumY)
			{
				position.y = minimumY / (cellSize * scaleFactor);
			}

		} while (IsCellOccupied(position));

		CreateGrass(position);
	}

	// Create wolf at its den.
	Vector2 wolfPosition;
	wolfPosition.x = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - ValueConfig::World::CellSize * scaleFactor * 2);
	wolfPosition.y = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f) / 2;

	float maxX = static_cast<float>(GetScreenWidth()) * 3.0f / 4.0f - ValueConfig::World::CellSize * scaleFactor;
	float maxY = static_cast<float>(GetScreenHeight()) * 2.0f / 3.0f - ValueConfig::World::CellSize * scaleFactor;

	wolfPosition.x = std::max(0.0f, std::min(wolfPosition.x, maxX));
	wolfPosition.y = std::max(minimumY, std::min(wolfPosition.y, maxY));


	for (int i = 0; i < wolfCount; i++)
	{
		CreateWolf(wolfPosition);
	}

	// Create initial sheep at random positions.
	for (int i = 0; i < sheepCount; i++)
	{
		Vector2 sheepPosition;

		do
		{
			sheepPosition.x = static_cast<float>(GetRandomValue(0, width - 1)) * cellSize * scaleFactor;
			sheepPosition.y = static_cast<float>(GetRandomValue(0, height - 1)) * cellSize * scaleFactor;

		} while (IsCellOccupied(sheepPosition));

		CreateSheep(sheepPosition);
	}
}

// Updates all grass entities and processes pending grass creation.
void World::Update(float deltaTime)
{
	// Update sheep poop instances.
	UpdateSheepPoop(deltaTime);

	// Update blood splatter instances.
	UpdateBloodSplatters(deltaTime);

	// Update and remove dead grass.
	for (auto it = grasses.begin(); it != grasses.end();)
	{
		if (!(*it)->IsAlive())
		{
			it = grasses.erase(it);
		}
		else
		{
			(*it)->Entity::Update(deltaTime);
			++it;
		}
	}

	// Update wolves
	for (auto it = wolves.begin(); it != wolves.end();)
	{
		if (!(*it)->IsAlive())
		{
			it = wolves.erase(it);
		}
		else
		{
			(*it)->Entity::Update(deltaTime);
			++it;
		}
	}

	std::vector<Vector2> newSheepPositions;

	// Update sheep
	for (auto it = sheeps.begin(); it != sheeps.end();)
	{
		if (!(*it)->IsAlive())
		{
			it = sheeps.erase(it);
		}
		else
		{
			(*it)->SetNewSheepPositions(&newSheepPositions);
			(*it)->Entity::Update(deltaTime);
			(*it)->SetNewSheepPositions(nullptr);

			++it;
		}
	}

	for (const auto& position : newSheepPositions)
	{
		CreateSheep(position);
	}

	// Create new grass from pending positions.
	for (const auto& positions : pendingGrassPositions)
	{
		CreateGrass(positions);
	}

	pendingGrassPositions.clear();
}

// Renders all grass entities in the world.
void World::Draw()
{
	for (const auto& grass : grasses)
	{
		grass->Draw();
	}

	for (const auto& poop : sheepPoopInstances)
	{
		float alpha = 1.0f - (poop.lifetime / SheepPoopInstance::maxLifetime);
		Color poopColor = { 139, 69, 19, static_cast<unsigned char>(255 * alpha) };

		DrawCircle(static_cast<int>(poop.position.x + ValueConfig::World::CellSize * scaleFactor / 2), static_cast<int>(poop.position.y + ValueConfig::World::CellSize * scaleFactor / 2), 5.0f * scaleFactor, poopColor);
	}

	for (const auto& splatter : bloodSplatters)
	{
		float alpha = 1.0f - (splatter.lifetime / BloodSplatterInstance::maxLifetime);

		Color bloodColor;

		if (splatter.size > 6.0f)
		{
			bloodColor = { 139, 0, 0, static_cast<unsigned char>(255 * alpha) };
		}
		else
		{
			bloodColor = { 220, 20, 20, static_cast<unsigned char>(255 * alpha) };
		}

		DrawCircle(static_cast<int>(splatter.position.x), static_cast<int>(splatter.position.y), splatter.size * scaleFactor, bloodColor);
	}

	for (const auto& wolf : wolves)
	{
		wolf->Draw();
	}

	for (const auto& sheep : sheeps)
	{
		sheep->Draw();
	}
}

// Checks if a cell is occupied by grass.
bool World::IsCellOccupied(Vector2 position) const
{
	for (const auto& grass : grasses)
	{
		if (grass->GetPosition().x == position.x && grass->GetPosition().y == position.y)
		{
			return true;
		}
	}

	return false;
}

// Adds spacing check and reduce spread chance.
bool World::IsAdjacentToGrass(Vector2 position) const
{
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}

			Vector2 checkPosition =
			{
				position.x + dx,position.y + dy
			};

			if (IsCellOccupied(checkPosition))
			{
				return true;
			}
		}
	}

	return false;
}

// Retrieves a list of valid neighboring cell positions.
std::vector<Vector2> World::GetNeighboringCells(Vector2 position) const
{
	std::vector<Vector2> neighbors;

	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}

			Vector2 newPosition =
			{
				position.x + dx,position.y + dy
			};

			if (newPosition.x >= 0 && newPosition.x < width && newPosition.y >= 0 && newPosition.y < height)
			{
				neighbors.push_back(newPosition);
			}
		}
	}

	return neighbors;
}

// Queues new grass creation in valid neighboring cells.
void World::SpreadSeeds(const Vector2& position)
{
	auto neighbors = GetNeighboringCells(position);

	for (const auto& neighborPositions : neighbors)
	{
		if (!IsCellOccupied(neighborPositions) && GetRandomValue(0, 100) < RuntimeConfig::GrassSpreadChance())
		{
			pendingGrassPositions.push_back(neighborPositions);
		}
	}
}

// Creates and initializes a new grass entity at the specified position.
void World::CreateGrass(Vector2 position)
{
	if (scaleFactor <= 0.5f) 
	{
		float minGridY = minY / (cellSize * scaleFactor);

		if (position.y < minGridY)
		{
			position.y = minGridY;
		}
	}

	auto grass = std::make_unique<GrassStateMachine>(this, position, scaleFactor);
	grass->Initialize();

	grasses.push_back(std::move(grass));
}

// Creates and initializes a new wolf entity at the specified position.
void World::CreateWolf(Vector2 position)
{
	auto wolf = std::make_unique<WolfStateMachine>(this, position, scaleFactor);
	wolf->Initialize();

	wolves.push_back(std::move(wolf));
}

// Creates and initializes a new sheep entity at the specified position.
void World::CreateSheep(Vector2 position)
{
	float sheepSize = ValueConfig::World::CellSize * scaleFactor;
	float minimumY = GetMinY();
	float rightMargin = 4.0f * scaleFactor;
	float bottomMargin = 4.0f * scaleFactor;

	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

	if (scaleFactor <= 0.5f && position.y < minimumY) 
	{
		position.y = minimumY;
	}

	position.x = std::max(0.0f, std::min(position.x, maxX));
	position.y = std::max(minimumY, std::min(position.y, maxY));

	auto sheep = std::make_unique<SheepStateMachine>(this, position, scaleFactor);
	sheep->Initialize();

	if (sheeps.size() > 0)
	{
		float closestDistance = RuntimeConfig::SheepGroupRadius() * scaleFactor;
		SheepStateMachine* closestSheep = nullptr;

		for (const auto& otherSheep : sheeps)
		{
			if (!otherSheep->IsAlive())
			{
				continue;
			}

			float dx = position.x - otherSheep->GetPosition().x;
			float dy = position.y - otherSheep->GetPosition().y;
			float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestSheep = otherSheep.get();
			}
		}

		if (closestSheep)
		{
			if (closestSheep->IsInGroup())
			{
				SheepStateMachine* leader = closestSheep->GetGroupLeader() ? closestSheep->GetGroupLeader() : closestSheep;

				if (leader->GetGroupSize() < RuntimeConfig::SheepMaxGroupSize())
				{
					leader->AddToGroup(sheep.get());
					sheep->SetGroupLeader(leader);
					sheep->SetCurrentState(SheepStateMachine::SheepState::WanderingInGroup);
					sheep->SwitchState(std::make_unique<WanderingInGroupState>(sheep.get()));
				}
			}
			else
			{
				sheep->AddToGroup(closestSheep);
				closestSheep->SetGroupLeader(sheep.get());
				closestSheep->SetCurrentState(SheepStateMachine::SheepState::WanderingInGroup);
				closestSheep->SwitchState(std::make_unique<WanderingInGroupState>(closestSheep));

				sheep->SetCurrentState(SheepStateMachine::SheepState::WanderingInGroup);
				sheep->SwitchState(std::make_unique<WanderingInGroupState>(sheep.get()));
			}
		}
	}

	sheeps.push_back(std::move(sheep));
}

// Adds a new sheep poop instance at the specified position.
void World::AddSheepPoop(Vector2 position)
{
	sheepPoopInstances.push_back({ position, 0.0f });
}

// Updates and removes sheep poop.
void World::UpdateSheepPoop(float deltaTime)
{
	for (auto it = sheepPoopInstances.begin(); it != sheepPoopInstances.end();)
	{
		it->lifetime += deltaTime;

		if (it->lifetime >= SheepPoopInstance::maxLifetime)
		{
			it = sheepPoopInstances.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// Checks if sheep poop is nearby within a specified radius.
bool World::IsSheepPoopNearby(Vector2 position, float radius) const
{
	float scaledRadius = radius * scaleFactor;
	float _minY = GetMinY();

	for (const auto& sheepPoop : sheepPoopInstances)
	{
		Vector2 poopPosition = sheepPoop.position;

		if (scaleFactor <= 0.5f && poopPosition.y < _minY) 
		{
			poopPosition.y = _minY;
		}

		float dx = position.x - poopPosition.x;
		float dy = position.y - poopPosition.y;
		float distanceSquared = dx * dx + dy * dy;

		if (distanceSquared < scaledRadius * scaledRadius)
		{
			return true;
		}
	}

	return false;
}

// Adds a new blood splatter instance at the specified position.
void World::AddBloodSplatter(Vector2 position)
{
	int numberOfSplatters = GetRandomValue(8, 15);

	for (int i = 0; i < numberOfSplatters; i++)
	{
		float offsetX = static_cast<float>(GetRandomValue(-60, 60)) * scaleFactor;
		float offsetY = static_cast<float>(GetRandomValue(-60, 60)) * scaleFactor;

		float size = static_cast<float>(GetRandomValue(2, 6)) * scaleFactor;

		Vector2 splatterPosition = { position.x + offsetX, position.y + offsetY };

		bloodSplatters.push_back({ splatterPosition, 0.0f, size });
	}
}

// Updates and removes blood splatters.
void World::UpdateBloodSplatters(float deltaTime)
{
	for (auto it = bloodSplatters.begin(); it != bloodSplatters.end();)
	{
		it->lifetime += deltaTime;

		if (it->lifetime >= BloodSplatterInstance::maxLifetime)
		{
			it = bloodSplatters.erase(it);
		}
		else
		{
			++it;
		}
	}
}