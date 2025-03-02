#include <Entities/Sheep/States/SheepStates.h>
#include <Entities/Sheep/SheepStateMachine.h>
#include <Utility/CoordinateUtils.h>
#include <World/World.h>


// Initializes a new sheep instance with its world reference and position
SheepStateMachine::SheepStateMachine(World* world, Vector2 position, float scaleFactor) : Entity(world, position), StateMachine(), world(world), position(position), isAlive(true), isReproducing(false), health(RuntimeConfig::SheepMaxHealth()), hunger(0.0f), fullness(0.0f), groupTimer(0.0f), groupLeader(nullptr), nearestWolf(nullptr), newSheepPositions(nullptr), scaleFactor(scaleFactor)
{
	moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f, static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };

	sheepTexture = LoadTexture("Assets/Sheep/TestSheep.png");

	float minY = world->GetMinY();

	if (scaleFactor <= 0.5f && this->position.y < minY)
	{
		this->position.y = minY;
	}
}

SheepStateMachine::~SheepStateMachine()
{
	LeaveGroup();

	UnloadTexture(sheepTexture);
}

// Sets up the initial state for the sheep and its starting conditions
void SheepStateMachine::Initialize()
{
	SwitchState(std::make_unique<WanderingAloneState>(this));
	currentSheepState = SheepState::WanderingAlone;
}

// Updates sheep's vital stats over time and checks for nearby sheep and wolves
void SheepStateMachine::Sense(float deltaTime)
{
	hunger = std::min(hunger + RuntimeConfig::SheepHungerIncreaseRate() * deltaTime, RuntimeConfig::SheepMaxHunger());

	if (hunger >= RuntimeConfig::SheepMaxHunger())
	{
		health = std::max(0.0f, health - RuntimeConfig::SheepHealthDecreaseRate() * deltaTime);

		if (health <= 0.0f)
		{
			health = 0.0f;
			Die();
		}
	}


	const auto& wolves = world->GetWolves();

	nearestWolf = nullptr;

	float closestDistance = RuntimeConfig::SheepWolfDetectionRadius() * scaleFactor;

	for (const auto& wolf : wolves)
	{
		float dx = position.x - wolf->GetPosition().x;
		float dy = position.y - wolf->GetPosition().y;

		float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

		if (distance < closestDistance)
		{
			closestDistance = distance;
			nearestWolf = wolf.get();
		}
	}

	if (groupCooldownTimer > 0.0f)
	{
		groupCooldownTimer -= deltaTime;
		if (groupCooldownTimer < 0.0f)
		{
			groupCooldownTimer = 0.0f;
		}
	}

	if (currentSheepState == SheepState::WanderingAlone && !nearestWolf && groupCooldownTimer <= 0.0f)
	{
		const auto& sheeps = world->GetSheep();

		float closestSheepDistance = RuntimeConfig::SheepGroupRadius() * scaleFactor;

		SheepStateMachine* closestSheep = nullptr;

		for (const auto& otherSheep : sheeps)
		{
			if (otherSheep.get() == this || !otherSheep->IsAlive())
			{
				continue;
			}

			float dx = position.x - otherSheep->GetPosition().x;
			float dy = position.y - otherSheep->GetPosition().y;
			float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

			if (distance < closestSheepDistance)
			{
				closestSheepDistance = distance;
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
					leader->AddToGroup(this);

					SetGroupLeader(leader);
					SwitchState(std::make_unique<WanderingInGroupState>(this));
				}
			}
			else
			{
				AddToGroup(closestSheep);

				closestSheep->SetGroupLeader(this);
				closestSheep->SwitchState(std::make_unique<WanderingInGroupState>(closestSheep));

				SwitchState(std::make_unique<WanderingInGroupState>(this));
			}
		}
	}

	else if (currentSheepState == SheepState::WanderingInGroup)
	{
		if (nearestWolf)
		{
			LeaveGroup();
			SwitchState(std::make_unique<RunningAwayState>(this));
		}

		if (IsGroupLeader())
		{
			std::vector<SheepStateMachine*> membersToRemove;

			for (auto member : groupMembers)
			{
				float dx = position.x - member->GetPosition().x;
				float dy = position.y - member->GetPosition().y;
				float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

				if (distance > (RuntimeConfig::SheepGroupRadius() * scaleFactor) * 1.5f)
				{
					membersToRemove.push_back(member);
				}
			}

			for (auto member : membersToRemove)
			{
				member->SetGroupLeader(nullptr);

				RemoveFromGroup(member);

				member->SwitchState(std::make_unique<WanderingAloneState>(member));
			}

			if (groupMembers.empty())
			{
				SwitchState(std::make_unique<WanderingAloneState>(this));
			}
		}
		else if (groupLeader)
		{
			float dx = position.x - groupLeader->GetPosition().x;
			float dy = position.y - groupLeader->GetPosition().y;

			float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

			if (distance > (RuntimeConfig::SheepGroupRadius() * scaleFactor) * 1.5f || !groupLeader->IsAlive())
			{
				LeaveGroup();
				SwitchState(std::make_unique<WanderingAloneState>(this));
			}
		}
	}
}

// Evaluates current state and determines next actions
void SheepStateMachine::Decide(float deltaTime)
{
	(void)deltaTime;

	if (nearestWolf && currentSheepState != SheepState::RunningAway)
	{
		SwitchState(std::make_unique<RunningAwayState>(this));
		return;
	}

	if (currentSheepState == SheepState::WanderingAlone || currentSheepState == SheepState::WanderingInGroup)
	{
		if (fullness >= RuntimeConfig::SheepMaxFullness())
		{
			SwitchState(std::make_unique<DefecatingState>(this));
			return;
		}

		if (hunger >= RuntimeConfig::SheepMaxHunger() * ValueConfig::Sheep::SheepHungerThreshold)
		{
			const auto& grasses = world->GetGrasses();
			const float grassDetectionRadius = RuntimeConfig::SheepGrassDetectionRadius() * scaleFactor;
			float minY = world->GetMinY();

			float closestDistance = grassDetectionRadius;
			GrassStateMachine* targetGrass = nullptr;

			for (const auto& grass : grasses)
			{
				if (grass->GetCurrentState() != GrassStateMachine::GrassState::FullyGrown || grass->IsBeingEaten())
				{
					continue;
				}

				Vector2 grassPosition = CoordinateUtils::GridToWorldPosition(grass->GetPosition(), world->GetScaleFactor());

				float drawY = grassPosition.y + minY;

				if (scaleFactor <= 0.5f && drawY < minY)
				{
					drawY = minY;
				}
				else
				{
					drawY = grassPosition.y + minY;
				}

				Vector2 correctedGrassPosition = { grassPosition.x, drawY };

				if (world->IsSheepPoopNearby(grassPosition, ValueConfig::World::CellSize * 2 * world->GetScaleFactor()))
				{
					continue;
				}

				float dx = position.x - grassPosition.x;
				float dy = position.y - grassPosition.y;
				float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

				if (distance < closestDistance)
				{
					closestDistance = distance;
					targetGrass = grass.get();
				}
			}

			if (targetGrass)
			{
				targetGrass->SetBeingEaten(true);

				Vector2 newPosition = CoordinateUtils::GridToWorldPosition(targetGrass->GetPosition(), world->GetScaleFactor());
				newPosition.y += world->GetMinY();

				float minimumY = world->GetMinY();
				float _scaleFactor = world->GetScaleFactor();
				float sheepSize = ValueConfig::World::CellSize * _scaleFactor;
				float rightMargin = 4.0f * _scaleFactor;
				float bottomMargin = 4.0f * _scaleFactor;

				float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
				float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

				newPosition.x = std::max(0.0f, std::min(newPosition.x, maxX));
				newPosition.y = std::max(minimumY, std::min(newPosition.y, maxY));

				position = newPosition;
				SwitchState(std::make_unique<SheepEatingState>(this));
				return;
			}
		}
	}

	if (currentSheepState == SheepState::WanderingInGroup && IsGroupLeader() && health > RuntimeConfig::SheepMaxHealth() * 0.5f && groupTimer >= ValueConfig::Sheep::ReproductionInterval)
	{
		const auto& members = GetGroupMembers();

		bool hasHealthyMembers = false;

		for (auto member : members)
		{
			if (member->GetHealth() > RuntimeConfig::SheepMaxHealth() * 0.5f)
			{
				hasHealthyMembers = true;
				break;
			}
		}

		if (hasHealthyMembers)
		{
			SwitchState(std::make_unique<ReproducingState>(this));
			return;
		}
	}
}

// Executes the current state's behavior each frame
void SheepStateMachine::Act(float deltaTime)
{
	StateMachine::Update(deltaTime);
}

// Renders the sheep on screen with color based on its current state
void SheepStateMachine::Draw()
{
	if (world->GetSimulation() && static_cast<Simulation*>(world->GetSimulation())->ShouldShowDetectionRadii())
	{
		Vector2 centerForOutline = { position.x + cellSize * scaleFactor / 2, position.y + cellSize * scaleFactor / 2 };

		Color skyBlueOutlineColor = { 102, 191, 255, 50 };
		Color magentaOutlineColor = { 255, 0, 255, 30 };
		Color redOutlineColor = { 230, 41, 55, 30 };

		DrawCircleLines(static_cast<int>(centerForOutline.x), static_cast<int>(centerForOutline.y), RuntimeConfig::SheepWolfDetectionRadius() * scaleFactor, skyBlueOutlineColor);
		DrawCircleLines(static_cast<int>(centerForOutline.x), static_cast<int>(centerForOutline.y), RuntimeConfig::SheepGrassDetectionRadius() * scaleFactor, magentaOutlineColor);
		DrawCircleLines(static_cast<int>(centerForOutline.x), static_cast<int>(centerForOutline.y), RuntimeConfig::SheepGroupRadius() * scaleFactor, redOutlineColor);
	}

	float scaledCellSize = cellSize * scaleFactor;
	float halfSize = scaledCellSize / 2;
	float minY = world->GetMinY();

	Vector2 drawPosition = position;

	if (scaleFactor <= 0.5f && drawPosition.y < minY)
	{
		drawPosition.y = minY;
	}

	Vector2 center = { drawPosition.x + halfSize, drawPosition.y + halfSize };

	Rectangle sourceRec = { 0.0f, 0.0f, static_cast<float>(sheepTexture.width), static_cast<float>(sheepTexture.height) };
	Rectangle destRec = { drawPosition.x, drawPosition.y, scaledCellSize, scaledCellSize };
	Vector2 origin = { 0.0f, 0.0f };

	DrawTexturePro(sheepTexture, sourceRec, destRec, origin, 0.0f, WHITE);

	float triangleOffset = 5.0f * scaleFactor;
	float triangleScale = 0.3f;
	float smallHalfSize = halfSize * triangleScale;

	Vector2 triangleCenter = { center.x, position.y + scaledCellSize + triangleOffset };

	Vector2 point1 = { triangleCenter.x + smallHalfSize, triangleCenter.y };
	Vector2 point2 = { triangleCenter.x - smallHalfSize, triangleCenter.y - smallHalfSize };
	Vector2 point3 = { triangleCenter.x - smallHalfSize, triangleCenter.y + smallHalfSize };

	Color sheepColor;

	if (currentSheepState == SheepState::WanderingInGroup)//|| (currentSheepState == SheepState::Eating && IsInGroup()))
	{
		sheepColor = { 25, 25, 153, 255 };
	}
	else if (currentSheepState == SheepState::RunningAway)
	{
		sheepColor = { 255, 0, 51, 255 };
	}
	else if (currentSheepState == SheepState::Defecating)
	{
		sheepColor = { 255, 128, 0, 255 };
	}
	else if (currentSheepState == SheepState::Eating)
	{
		sheepColor = { 102, 51, 153, 255 };
	}
	else if (currentSheepState == SheepState::Reproducing && isReproducing)
	{
		sheepColor = PINK;
	}
	else
	{
		sheepColor = { 135, 206, 250, 255 };
	}

	DrawTriangle(point1, point2, point3, sheepColor);

	DrawStatusBars(center, health, hunger);


	if (currentSheepState == SheepState::WanderingInGroup || currentSheepState == SheepState::Reproducing || (currentSheepState == SheepState::Eating && IsInGroup()) || (currentSheepState == SheepState::Defecating && IsInGroup()))
	{
		if (IsGroupLeader())
		{
			for (auto member : groupMembers)
			{
				Vector2 memberCenter = { member->GetPosition().x + halfSize, member->GetPosition().y + halfSize };

				Color lineColor;

				if (currentSheepState == SheepState::Reproducing && member->GetCurrentState() == SheepState::Reproducing)
				{
					lineColor = PINK;
				}
				else if (currentSheepState == SheepState::Reproducing || member->GetCurrentState() == SheepState::Reproducing)
				{
					lineColor = { 220, 120, 220, 255 };
				}
				else if (currentSheepState == SheepState::Eating || member->GetCurrentState() == SheepState::Eating)
				{
					lineColor = { 102, 51, 153, 255 };
				}
				else if (currentSheepState == SheepState::Defecating || member->GetCurrentState() == SheepState::Defecating)
				{
					lineColor = { 255, 128, 0, 255 };
				}
				else
				{
					lineColor = { 25, 25, 153, 255 };
				}

				DrawLine(static_cast<int>(center.x), static_cast<int>(center.y), static_cast<int>(memberCenter.x), static_cast<int>(memberCenter.y), lineColor);
			}
		}
		else if (groupLeader)
		{
			Vector2 leaderCenter = { groupLeader->GetPosition().x + halfSize, groupLeader->GetPosition().y + halfSize };

			Color lineColor;

			if (currentSheepState == SheepState::Reproducing && groupLeader->GetCurrentState() == SheepState::Reproducing)
			{
				lineColor = PINK;
			}
			else if (currentSheepState == SheepState::Reproducing || groupLeader->GetCurrentState() == SheepState::Reproducing)
			{
				lineColor = { 220, 120, 220, 255 };
			}
			else if (currentSheepState == SheepState::Eating || groupLeader->GetCurrentState() == SheepState::Eating)
			{
				lineColor = GREEN;
			}
			else if (currentSheepState == SheepState::Defecating || groupLeader->GetCurrentState() == SheepState::Defecating)
			{
				lineColor = ORANGE;
			}
			else
			{
				lineColor = BLUE;
			}

			DrawLine(static_cast<int>(center.x), static_cast<int>(center.y), static_cast<int>(leaderCenter.x), static_cast<int>(leaderCenter.y), lineColor);
		}
	}
}

// Draws status bar for sheep's health and hunger.
void SheepStateMachine::DrawStatusBars(Vector2 barPosition, float healthValue, float hungerValue)
{
	const float barWidth = 30.0f * scaleFactor;
	const float barHeight = 4.0f * scaleFactor;
	const float barSpacing = 2.0f * scaleFactor;
	const float barY = barPosition.y - cellSize * scaleFactor / 2 - 10.0f * scaleFactor;


	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY), static_cast<int>(barWidth), static_cast<int>(barHeight), GRAY);

	float healthPercentage = healthValue / RuntimeConfig::SheepMaxHealth();

	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY), static_cast<int>(barWidth * healthPercentage), static_cast<int>(barHeight), RED);

	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY + barHeight + barSpacing), static_cast<int>(barWidth), static_cast<int>(barHeight), GRAY);

	float hungerPercentage = hungerValue / RuntimeConfig::SheepMaxHunger();

	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY + barHeight + barSpacing), static_cast<int>(barWidth * (1.0f - hungerPercentage)), static_cast<int>(barHeight), GREEN);
}

// Adds sheep to a potential group if it's not full and the sheep is not the same as the current one.
void SheepStateMachine::AddToGroup(SheepStateMachine* sheep)
{
	if (groupMembers.size() < RuntimeConfig::SheepMaxGroupSize() && sheep != this)
	{
		groupMembers.insert(sheep);
	}
}

// Removes sheep from the group.
void SheepStateMachine::RemoveFromGroup(SheepStateMachine* sheep)
{
	groupMembers.erase(sheep);
}

// Leaves the current group and sets the group leader to null.
void SheepStateMachine::LeaveGroup()
{
	if (groupLeader && groupLeader != this)
	{
		groupLeader->RemoveFromGroup(this);
	}

	for (auto member : groupMembers)
	{
		if (member->GetGroupLeader() == this)
		{
			member->SetGroupLeader(nullptr);
			member->SwitchState(std::make_unique<WanderingAloneState>(member));
		}
	}

	groupMembers.clear();
	groupLeader = nullptr;
}

// Reuests new sheep to be added after reproduction state has happened.
void SheepStateMachine::RequestNewSheep(Vector2 newPosition)
{
	if (newSheepPositions)
	{
		newSheepPositions->push_back(newPosition);
	}
}

// Sets the new position for the sheep, ensuring it stays within the world bounds.
void SheepStateMachine::SetPosition(Vector2 newPosition)
{
	float minY = world->GetMinY();
	float sheepSize = ValueConfig::World::CellSize * scaleFactor;
	float rightMargin = 4.0f * scaleFactor;
	float bottomMargin = 4.0f * scaleFactor;

	float topOffset = 0.0f;

	if (scaleFactor <= 0.5f)
	{
		topOffset = 15.0f * scaleFactor;
	}
	else if (scaleFactor >= 0.5f && scaleFactor <= 0.75f)
	{
		topOffset = 10.0f * scaleFactor;
	}

	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

	newPosition.x = std::max(0.0f, std::min(newPosition.x, maxX));
	newPosition.y = std::max(minY + topOffset, std::min(newPosition.y, maxY));

	position = newPosition;
}