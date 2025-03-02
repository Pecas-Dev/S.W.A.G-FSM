#include <Entities/Wolf/States/WolfStates.h>
#include <Entities/Wolf/WolfStateMachine.h>
#include <Utility/SimulationConfig.h>
#include <Utility/ValueConfig.h>
#include <World/World.h>

#include <memory>


// Initializes the sleeping state with a reset timer.
SleepingState::SleepingState(WolfStateMachine* stateMachine) : WolfBaseState(stateMachine), sleepTimer(0.0f)
{
}

// Sets up initial sleeping state parameters and updates wolf's visual state.
void SleepingState::Enter()
{
	Simulation* _simulation_ = wolfStateMachine->GetWorld()->GetSimulation();

	if (_simulation_)
	{
		_simulation_->AddConsoleMessage("Wolf entered SLEEPING state\n");
	}

	sleepTimer = 0.0f;
	wolfStateMachine->SetCurrentState(WolfStateMachine::WolfState::Sleeping);
	wolfStateMachine->SetTargetSheep(nullptr);
}

// Monitors hunger level and transitions to roaming when threshold is reached.
void SleepingState::Tick(float deltaTime)
{
	sleepTimer += deltaTime;

	if (wolfStateMachine->GetHunger() >= wolfStateMachine->GetHungerThreshold())
	{
		wolfStateMachine->SwitchState(std::make_unique<RoamingState>(wolfStateMachine));
	}
}

// Cleanup when exiting sleeping state.
void SleepingState::Exit()
{
}

// Initializes the roaming state with movement parameters.
RoamingState::RoamingState(WolfStateMachine* stateMachine) : WolfBaseState(stateMachine), movementTimer(0.0f)
{
	moveDirection = { 0.0f, 0.0f };
}

// Sets up roaming behavior and initial random movement direction.
void RoamingState::Enter()
{
	Simulation* _simulation_ = wolfStateMachine->GetWorld()->GetSimulation();

	if (_simulation_)
	{
		_simulation_->AddConsoleMessage("Wolf entered ROAMING state\n");
	}

	wolfStateMachine->SetCurrentState(WolfStateMachine::WolfState::Roaming);
	moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };

	wolfStateMachine->SetTargetSheep(nullptr);
}

// Updates wolf position and manages random movement patterns.
void RoamingState::Tick(float deltaTime)
{
	movementTimer += deltaTime;

	if (movementTimer >= timeToChangeDirection)
	{
		moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
		movementTimer = 0.0f;
	}

	Vector2 currentPosition = wolfStateMachine->GetPosition();

	float scaleFactor = wolfStateMachine->GetWorld()->GetScaleFactor();
	float scaledSpeed = RuntimeConfig::WolfRoamSpeed() * scaleFactor;

	currentPosition.x += moveDirection.x * scaledSpeed * deltaTime;
	currentPosition.y += moveDirection.y * scaledSpeed * deltaTime;

	// Keep wolf within screen bounds
	float minY = wolfStateMachine->GetWorld()->GetMinY();
	float cellSize = ValueConfig::World::CellSize * scaleFactor;
	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - cellSize);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - cellSize);

	currentPosition.x = std::max(0.0f, std::min(currentPosition.x, maxX));
	currentPosition.y = std::max(minY, std::min(currentPosition.y, maxY));

	wolfStateMachine->SetPosition(currentPosition);
}

// Cleanup when exiting roaming state.
void RoamingState::Exit()
{
}

// Initializes hunting state for pursuing Sheep.
HuntingState::HuntingState(WolfStateMachine* stateMachine) : WolfBaseState(stateMachine), attackTimer(0.0f), targetUpdateTimer(0.0f), stateCheckTimer(0.0f)
{
}

// Sets up hunting behavior and updates wolf's visual state.
void HuntingState::Enter()
{
	Simulation* _simulation_ = wolfStateMachine->GetWorld()->GetSimulation();

	if (_simulation_)
	{
		printf("Wolf entered HUNTING state\n");
	}

	wolfStateMachine->SetCurrentState(WolfStateMachine::WolfState::Hunting);
	targetUpdateTimer = 0.0f;
	stateCheckTimer = 0.0f;
}

// Manages Sheep's pursuit and attack behavior.
void HuntingState::Tick(float deltaTime)
{
	SheepStateMachine* targetSheep = wolfStateMachine->GetTargetSheep();

	attackTimer += deltaTime;
	targetUpdateTimer += deltaTime;
	stateCheckTimer += deltaTime;

	if (stateCheckTimer >= stateValidationInterval)
	{
		stateCheckTimer = 0.0f;

		if (!targetSheep || !targetSheep->IsAlive())
		{
			wolfStateMachine->SetTargetSheep(nullptr);
			wolfStateMachine->SwitchState(std::make_unique<RoamingState>(wolfStateMachine));
			return;
		}
	}

	if (!targetSheep || !targetSheep->IsAlive())
	{
		return;
	}

	float currentStamina = wolfStateMachine->GetStamina();
	currentStamina = std::max(0.0f, currentStamina - RuntimeConfig::WolfStaminaDrainRate() * deltaTime);

	wolfStateMachine->SetStamina(currentStamina);

	Vector2 currentPosition = wolfStateMachine->GetPosition();
	Vector2 targetPosition = targetSheep->GetPosition();

	float dx = targetPosition.x - currentPosition.x;
	float dy = targetPosition.y - currentPosition.y;
	float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

	float scaleFactor = wolfStateMachine->GetWorld()->GetScaleFactor();
	float scaledCellSize = ValueConfig::World::CellSize * scaleFactor;

	if (distance < scaledCellSize)
	{
		if (attackTimer >= attackInterval)
		{
			float newHealth = targetSheep->GetHealth() - RuntimeConfig::WolfAttackDamage();
			newHealth = std::max(0.0f, newHealth);
			targetSheep->SetHealth(newHealth);

			wolfStateMachine->GetWorld()->AddBloodSplatter(targetSheep->GetPosition());
			attackTimer = 0.0f;

			if (newHealth <= 0.0f)
			{
				wolfStateMachine->SwitchState(std::make_unique<WolfEatingState>(wolfStateMachine));
				return;
			}
		}
	}
	else
	{
		if (distance > 0)
		{
			dx = dx / distance;
			dy = dy / distance;

			float effectiveHuntSpeed = RuntimeConfig::WolfHuntSpeed() * scaleFactor;

			if (wolfStateMachine->IsTired())
			{
				effectiveHuntSpeed *= ValueConfig::Wolf::TiredSpeedMultiplier;
			}

			if (GetRandomValue(0, 100) < 10)
			{
				dx += static_cast<float>(GetRandomValue(-15, 15)) / 100.0f;
				dy += static_cast<float>(GetRandomValue(-15, 15)) / 100.0f;

				float newLength = static_cast<float>(sqrt(dx * dx + dy * dy));

				if (newLength > 0)
				{
					dx /= newLength;
					dy /= newLength;
				}
			}

			Vector2 separationDirection = CalculateWolfSeparation();

			dx = dx * 0.7f + separationDirection.x * 0.3f;
			dy = dy * 0.7f + separationDirection.y * 0.3f;

			float newLength = static_cast<float>(sqrt(dx * dx + dy * dy));

			if (newLength > 0)
			{
				dx /= newLength;
				dy /= newLength;
			}

			dx *= effectiveHuntSpeed * deltaTime;
			dy *= effectiveHuntSpeed * deltaTime;

			currentPosition.x += dx;
			currentPosition.y += dy;

			currentPosition.x = std::max(0.0f, std::min(currentPosition.x, static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - scaledCellSize)));
			currentPosition.y = std::max(wolfStateMachine->GetWorld()->GetMinY(), std::min(currentPosition.y, static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - scaledCellSize)));

			wolfStateMachine->SetPosition(currentPosition);
		}
	}
}

// Helper Function to calculate separation vector to avoid overlapping with other wolves.
Vector2 HuntingState::CalculateWolfSeparation()
{
	Vector2 separationDirection = { 0.0f, 0.0f };
	Vector2 wolfPosition = wolfStateMachine->GetPosition();

	float scaleFactor = wolfStateMachine->GetWorld()->GetScaleFactor();
	float scaledSeparationMinDistance = separationMinDistance * scaleFactor;
	float scaledSeparationMinDistanceSquared = scaledSeparationMinDistance * scaledSeparationMinDistance;

	const auto& wolves = wolfStateMachine->GetWorld()->GetWolves();

	int neighborCount = 0;

	for (const auto& otherWolf : wolves)
	{
		if (otherWolf.get() == wolfStateMachine)
		{
			continue;
		}

		Vector2 otherPositions = otherWolf->GetPosition();

		float dx = wolfPosition.x - otherPositions.x;
		float dy = wolfPosition.y - otherPositions.y;
		float distanceSquared = dx * dx + dy * dy;

		if (distanceSquared < scaledSeparationMinDistanceSquared)
		{
			float distance = static_cast<float>(sqrt((distanceSquared)));
			float strength = 1.0f - (distance / scaledSeparationMinDistance);

			if (distance > 0)
			{
				separationDirection.x += (dx / distance) * strength;
				separationDirection.y += (dy / distance) * strength;
			}

			neighborCount++;
		}
	}

	if (neighborCount > 0)
	{
		float length = static_cast<float>(sqrt(separationDirection.x * separationDirection.x + separationDirection.y * separationDirection.y));

		if (length > 0)
		{
			separationDirection.x /= length;
			separationDirection.y /= length;
		}
	}

	return separationDirection;
}

// Cleanup when exiting hunting state.
void HuntingState::Exit()
{
}

// Initializes eating state with consumption timer.
WolfEatingState::WolfEatingState(WolfStateMachine* stateMachine) : WolfBaseState(stateMachine), eatingTimer(0.0f)
{
}

// Sets up eating behavior and updates wolf's visual state.
void WolfEatingState::Enter()
{
	Simulation* _simulation_ = wolfStateMachine->GetWorld()->GetSimulation();

	if (_simulation_)
	{
		printf("Wolf entered EATING state\n");
	}

	eatingTimer = 0.0f;
	wolfStateMachine->SetCurrentState(WolfStateMachine::WolfState::Eating);
}

// Manages feeding duration and hunger satisfaction.
void WolfEatingState::Tick(float deltaTime)
{
	eatingTimer += deltaTime;

	if (eatingTimer >= timeToEat)
	{
		wolfStateMachine->SetHunger(0.0f);
		wolfStateMachine->SetTargetSheep(nullptr);
		wolfStateMachine->SwitchState(std::make_unique<ReturnToDenState>(wolfStateMachine));
	}
}

// Cleanup when exiting eating state.
void WolfEatingState::Exit()
{
	wolfStateMachine->SetTargetSheep(nullptr);
}

// Initializes return to den state for moving back home.
ReturnToDenState::ReturnToDenState(WolfStateMachine* stateMachine) : WolfBaseState(stateMachine)
{
}

// Sets up return behavior and updates wolf's visual state.
void ReturnToDenState::Enter()
{
	Simulation* _simulation_ = wolfStateMachine->GetWorld()->GetSimulation();

	if (_simulation_)
	{
		printf("Wolf entered RETURN-TO-DEN state\n");
	}

	wolfStateMachine->SetCurrentState(WolfStateMachine::WolfState::ReturnToDen);
	wolfStateMachine->SetTargetSheep(nullptr);
}

// Manages wolf's movement back to its den.
void ReturnToDenState::Tick(float deltaTime)
{
	float scaleFactor = wolfStateMachine->GetWorld()->GetScaleFactor();
	float cellSize = ValueConfig::World::CellSize * scaleFactor;
	float scaledReturnSpeed = returnSpeed * scaleFactor;
	float scaledDenProximityThreshold = denProximityThreshold * scaleFactor;

	Vector2 denPosition;
	denPosition.x = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - cellSize * 2);
	denPosition.y = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f) / 2;

	float minY = wolfStateMachine->GetWorld()->GetMinY();
	float maxX = static_cast<float>(GetScreenWidth()) * 3.0f / 4.0f - cellSize;
	float maxY = static_cast<float>(GetScreenHeight()) * 2.0f / 3.0f - cellSize;

	denPosition.x = std::max(0.0f, std::min(denPosition.x, maxX));
	denPosition.y = std::max(minY, std::min(denPosition.y, maxY));

	Vector2 wolfPosition = wolfStateMachine->GetPosition();

	float dx = denPosition.x - wolfPosition.x;
	float dy = denPosition.y - wolfPosition.y;
	float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

	if (distance < scaledDenProximityThreshold)
	{
		wolfStateMachine->SetPosition(denPosition);
		wolfStateMachine->SwitchState(std::make_unique<SleepingState>(wolfStateMachine));
		return;
	}

	dx = (dx / distance) * scaledReturnSpeed * deltaTime;
	dy = (dy / distance) * scaledReturnSpeed * deltaTime;

	wolfPosition.x += dx;
	wolfPosition.y += dy;

	wolfStateMachine->SetPosition(wolfPosition);
}

// Cleanup when exiting return to den state.
void ReturnToDenState::Exit()
{
}