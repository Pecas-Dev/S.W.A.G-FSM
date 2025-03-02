#include <Entities/Wolf/States/WolfStates.h>
#include <Entities/Wolf/WolfStateMachine.h>
#include <Utility/SimulationConfig.h>
#include <Utility/ValueConfig.h>
#include <World/World.h>


// Initializes a new wolf instance with its world reference and position.
WolfStateMachine::WolfStateMachine(World* world, Vector2 position, float scaleFactor) : Entity(world, position), StateMachine(), world(world), position(position), isAlive(true), hunger(0.0f), stamina(RuntimeConfig::WolfStaminaMax()), targetSheep(nullptr), scaleFactor(scaleFactor)
{
	float minY = world->GetMinY();
	position.x = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f) - cellSize * scaleFactor * 2;
	position.y = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f) / 2;

	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - cellSize * scaleFactor);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - cellSize * scaleFactor);

	position.x = std::max(0.0f, std::min(position.x, maxX));
	position.y = std::max(minY, std::min(position.y, maxY));

	wolfTexture = LoadTexture("Assets/Wolf/TestWolf.png");
}

WolfStateMachine::~WolfStateMachine()
{
	UnloadTexture(wolfTexture);
}

// Sets up the initial state for the wolf and its starting conditions.
void WolfStateMachine::Initialize()
{
	SwitchState(std::make_unique<SleepingState>(this));
	currentWolfState = WolfState::Sleeping;
}

// Updates wolf's vital stats over time and checks for nearby sheep.
void WolfStateMachine::Sense(float deltaTime)
{
	if (currentWolfState != WolfState::ReturnToDen)
	{
		hunger = std::min(hunger + RuntimeConfig::WolfHungerIncreaseRate() * deltaTime, RuntimeConfig::WolfMaxHunger());
	}

	if (currentWolfState != WolfState::Hunting)
	{
		stamina = std::min(stamina + ValueConfig::Wolf::StaminaRecoveryRate * deltaTime, RuntimeConfig::WolfStaminaMax());
	}

	if (targetSheep)
	{
		if (!targetSheep->IsAlive())
		{
			if (Simulation* _simulation_ = dynamic_cast<Simulation*>(GetWorld()->GetSimulation()))
			{
				printf("Wolf clearing target - sheep is dead\n");
			}

			targetSheep = nullptr;

			if (currentWolfState == WolfState::Hunting)
			{
				if (Simulation* _simulation_ = dynamic_cast<Simulation*>(GetWorld()->GetSimulation()))
				{
					printf("Wolf returning to roaming - target is gone\n");
				}

				SwitchState(std::make_unique<RoamingState>(this));
			}
		}
	}

	targetingTimer += deltaTime;

	if (!targetSheep && currentWolfState != WolfState::Eating && currentWolfState != WolfState::ReturnToDen && targetingTimer >= targetAcquisitionDelay)
	{
		FindNewTarget();
		targetingTimer = 0.0f;
	}
}

// Evaluates current state and determines next actions.
void WolfStateMachine::Decide(float deltaTime)
{
	(void)deltaTime;

	if (currentWolfState == WolfState::Roaming && targetSheep != nullptr)
	{
		SwitchState(std::make_unique<HuntingState>(this));
	}
	else if (currentWolfState == WolfState::Hunting && targetSheep == nullptr)
	{
		SwitchState(std::make_unique<RoamingState>(this));
	}

	if (currentWolfState == WolfState::Hunting && stamina <= 0.1f)
	{
		SwitchState(std::make_unique<RoamingState>(this));
	}
}

// Executes the current state's behavior each frame.
void WolfStateMachine::Act(float deltaTime)
{
	StateMachine::Update(deltaTime);
}

// Renders the wolf on screen with color based on its current state.
void WolfStateMachine::Draw()
{
	Color wolfColor;

	Color otherStates = { 196, 180, 84, 255 };
	Color tiredColor = { 160, 160, 160, 255 };

	if (world->GetSimulation() && static_cast<Simulation*>(world->GetSimulation())->ShouldShowDetectionRadii())
	{
		Vector2 centerForOutline = { position.x + cellSize * scaleFactor / 2, position.y + cellSize * scaleFactor / 2 };
		Color outlineColor = { 0, 0, 0, 20 };

		DrawCircleLines(static_cast<int>(centerForOutline.x), static_cast<int>(centerForOutline.y), RuntimeConfig::WolfSheepDetectionRadius() * scaleFactor, outlineColor);
	}

	Vector2 drawPosition = position;

	float minY = world->GetMinY();

	if (scaleFactor <= 0.5f && drawPosition.y < minY)
	{
		drawPosition.y = minY;
	}

	float scaledCellSize = cellSize * scaleFactor;
	Vector2 center = { drawPosition.x + scaledCellSize / 2, drawPosition.y + scaledCellSize / 2 };

	Rectangle sourceRec = { 0.0f, 0.0f, static_cast<float>(wolfTexture.width), static_cast<float>(wolfTexture.height) };
	Rectangle destRec = { drawPosition.x, drawPosition.y, scaledCellSize, scaledCellSize };
	Vector2 origin = { 0.0f, 0.0f };

	DrawTexturePro(wolfTexture, sourceRec, destRec, origin, 0.0f, WHITE);

	switch (currentWolfState)
	{
	case WolfState::Sleeping:
		wolfColor = BLACK;
		break;
	case WolfState::Roaming:
		wolfColor = { 255, 255, 0, 255 };
		break;
	case WolfState::Hunting:
		if (IsTired())
		{
			wolfColor = tiredColor;
		}
		else
		{
			wolfColor = { 128, 0, 0, 255 };
		}
		break;
	case WolfState::Eating:
		wolfColor = { 4, 102, 100, 255 };
		break;
	case WolfState::ReturnToDen:
		wolfColor = { 128, 191, 51, 255 };
		break;
	default:
		wolfColor = { 128, 128, 128, 255 };
		break;
	}

	float circleOffset = 5.0f * scaleFactor;
	float circleScale = 0.3f;
	float smallRadius = scaledCellSize / 2 * circleScale;

	Vector2 circleCenter = { center.x, drawPosition.y + scaledCellSize + circleOffset };

	DrawCircle(static_cast<int>(circleCenter.x), static_cast<int>(circleCenter.y), smallRadius, wolfColor);


	Vector2 centerForStaminaBar = { drawPosition.x + scaledCellSize / 2, drawPosition.y + scaledCellSize / 2 };

	DrawStatusBars(centerForStaminaBar, stamina);


	DrawHuntingLine();
}

// Helper Function that Finds a new target sheep that isn't being hunted by another wolf.
void WolfStateMachine::FindNewTarget()
{
	const auto& sheep = world->GetSheep();
	const auto& wolves = world->GetWolves();

	float closestDistance = RuntimeConfig::WolfSheepDetectionRadius() * scaleFactor;

	SheepStateMachine* bestTarget = nullptr;

	for (const auto& target : sheep)
	{
		if (!target->IsAlive())
		{
			continue;
		}

		bool beingHunted = false;

		for (const auto& otherWolf : wolves)
		{
			if (otherWolf.get() != this && otherWolf->GetTargetSheep() == target.get())
			{
				beingHunted = true;
				break;
			}
		}

		if (beingHunted)
		{
			continue;
		}

		Vector2 sheepPosition = target->GetPosition();
		Vector2 wolfPosition = GetPosition();

		float dx = wolfPosition.x - sheepPosition.x;
		float dy = wolfPosition.y - sheepPosition.y;
		float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

		if (distance < closestDistance)
		{
			closestDistance = distance;
			bestTarget = target.get();
		}
	}

	if (bestTarget)
	{
		targetSheep = bestTarget;
	}
}

// Helper Function that Draws a line from the wolf to the sheep it is hunting.
void WolfStateMachine::DrawHuntingLine()
{
	if (currentWolfState == WolfState::Hunting && targetSheep != nullptr && targetSheep->IsAlive())
	{
		Vector2 drawPosition = position;

		float minY = world->GetMinY();

		if (scaleFactor <= 0.5f && drawPosition.y < minY)
		{
			drawPosition.y = minY;
		}

		Vector2 wolfCenter = { drawPosition.x + cellSize * scaleFactor / 2, drawPosition.y + cellSize * scaleFactor / 2 };
		Vector2 sheepPosition = targetSheep->GetPosition();

		if (scaleFactor <= 0.5f && sheepPosition.y < minY)
		{
			sheepPosition.y = minY;
		}

		Vector2 sheepCenter = { sheepPosition.x + cellSize * scaleFactor / 2, sheepPosition.y + cellSize * scaleFactor / 2 };

		const int segments = 20;
		const float segmentLength = 5.0f * scaleFactor;
		Vector2 direction = { sheepCenter.x - wolfCenter.x, sheepCenter.y - wolfCenter.y };

		float distance = static_cast<float>(sqrt(direction.x * direction.x + direction.y * direction.y));

		if (distance > 0)
		{
			direction.x /= distance;
			direction.y /= distance;

			for (int i = 0; i < segments; i++)
			{
				if (i % 2 == 0)
				{
					float startDistance = i * segmentLength;

					if (startDistance < distance)
					{
						Vector2 startPosition =
						{
							wolfCenter.x + direction.x * startDistance,
							wolfCenter.y + direction.y * startDistance
						};

						float endDistance = std::min((i + 1) * segmentLength, distance);

						Vector2 endPosition =
						{
							wolfCenter.x + direction.x * endDistance,
							wolfCenter.y + direction.y * endDistance
						};

						DrawLineEx(startPosition, endPosition, 2.0f * scaleFactor, { 128, 0, 0, 255 });
					}
				}
			}
		}
	}
}

// Sets the new position for the wolf, ensuring it stays within the world bounds.
void WolfStateMachine::SetPosition(Vector2 newPosition)
{
	float minY = world->GetMinY();
	float wolfSize = ValueConfig::World::CellSize * scaleFactor;
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

	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - wolfSize - rightMargin);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - wolfSize - bottomMargin);

	newPosition.x = std::max(0.0f, std::min(newPosition.x, maxX));
	newPosition.y = std::max(minY + topOffset, std::min(newPosition.y, maxY));

	position = newPosition;
}

// Draws the wolf's stamina and hunger bars on screen.
void WolfStateMachine::DrawStatusBars(Vector2 barPosition, float staminaValue)
{
	const float barWidth = 30.0f * scaleFactor;
	const float barHeight = 4.0f * scaleFactor;
	const float barY = barPosition.y - cellSize * scaleFactor / 2 - 10.0f * scaleFactor;

	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY), static_cast<int>(barWidth), static_cast<int>(barHeight), GRAY);

	float staminaPercentage = staminaValue / RuntimeConfig::WolfStaminaMax();

	DrawRectangle(static_cast<int>(barPosition.x - barWidth / 2), static_cast<int>(barY), static_cast<int>(barWidth * staminaPercentage), static_cast<int>(barHeight), { 128, 0, 32, 255 });
}