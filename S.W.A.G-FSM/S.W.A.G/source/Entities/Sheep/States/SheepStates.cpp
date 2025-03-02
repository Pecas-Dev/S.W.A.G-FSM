#include <Entities/Sheep/States/SheepStates.h>
#include <Entities/Sheep/SheepStateMachine.h>
#include <Utility/CoordinateUtils.h>
#include <World/World.h>

#include <memory>


// Initializes the wandering alone state with movement parameters.
WanderingAloneState::WanderingAloneState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine), movementTimer(0.0f)
{
	moveDirection = { 0.0f, 0.0f };
}

// Sets up initial wandering behavior and random movement direction.
void WanderingAloneState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		_simulation_->AddConsoleMessage("Sheep entered WANDERING ALONE state\n");
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::WanderingAlone);
	moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
}

// Updates sheep position and manages random movement patterns.
void WanderingAloneState::Tick(float deltaTime)
{
	movementTimer += deltaTime;

	if (movementTimer >= timeToChangeDirection)
	{
		moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
		movementTimer = 0.0f;
	}

	Vector2 currentPosition = sheepStateMachine->GetPosition();
	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float scaledSpeed = moveSpeed * scaleFactor;

	currentPosition.x += moveDirection.x * scaledSpeed * deltaTime;
	currentPosition.y += moveDirection.y * scaledSpeed * deltaTime;

	World* world = sheepStateMachine->GetWorld();
	float minimumY = world->GetMinY();

	if (scaleFactor <= 0.5f && currentPosition.y < minimumY)
	{
		currentPosition.y = minimumY;
	}

	float sheepSize = ValueConfig::World::CellSize * scaleFactor;
	float rightMargin = 4.0f * scaleFactor;
	float bottomMargin = 4.0f * scaleFactor;

	float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
	float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

	currentPosition.x = std::max(0.0f, std::min(currentPosition.x, maxX));
	currentPosition.y = std::max(minimumY, std::min(currentPosition.y, maxY));

	sheepStateMachine->SetPosition(currentPosition);
}

// Cleanup when exiting wandering alone state.
void WanderingAloneState::Exit()
{
}

// Initializes the wandering in group state with group movement parameters.
WanderingInGroupState::WanderingInGroupState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine), movementTimer(0.0f)
{
	moveDirection = { 0.0f, 0.0f };
}

// Sets up group wandering behavior and updates sheep's visual state.
void WanderingInGroupState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		printf("Sheep entered WANDERING IN GROUP state\n");
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::WanderingInGroup);

	moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f, static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
}

// Manages cohesive movement behavior for grouped sheep.
void WanderingInGroupState::Tick(float deltaTime)
{
	movementTimer += deltaTime;
	sheepStateMachine->UpdateGroupTimer(deltaTime);

	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float scaledSpeed = moveSpeed * scaleFactor;
	float scaledMinGroupDistance = ValueConfig::Sheep::MinGroupDistance * scaleFactor;
	float scaledMaxGroupDistance = ValueConfig::Sheep::MaxGroupDistance * scaleFactor;
	float minimumY = sheepStateMachine->GetWorld()->GetMinY();

	if (sheepStateMachine->GetGroupTimer() >= ValueConfig::Sheep::ReproductionInterval && sheepStateMachine->IsGroupLeader() && sheepStateMachine->GetHealth() > RuntimeConfig::SheepMaxHealth() * 0.5f)
	{
		sheepStateMachine->ResetGroupTimer();

		const auto& members = sheepStateMachine->GetGroupMembers();

		std::vector<SheepStateMachine*> candidateSheep;
		candidateSheep.push_back(sheepStateMachine);

		for (auto member : members)
		{
			if (member->GetHealth() > RuntimeConfig::SheepMaxHealth() * 0.5f)
			{
				candidateSheep.push_back(member);
			}
		}

		for (int i = 0; i < static_cast<int>(candidateSheep.size()); i++)
		{
			int j = GetRandomValue(0, static_cast<int>(candidateSheep.size()) - 1);

			if (i != j)
			{
				std::swap(candidateSheep[i], candidateSheep[j]);
			}
		}

		int pairCount = static_cast<int>(candidateSheep.size()) / 2;

		for (int i = 0; i < pairCount; i++)
		{
			candidateSheep[i * 2]->SetIsReproducing(true);
			candidateSheep[i * 2]->SetCurrentState(SheepStateMachine::SheepState::Reproducing);

			if (i * 2 + 1 < static_cast<int>(candidateSheep.size()))
			{
				candidateSheep[i * 2 + 1]->SetIsReproducing(true);
				candidateSheep[i * 2 + 1]->SetCurrentState(SheepStateMachine::SheepState::Reproducing);
			}
		}

		if (!sheepStateMachine->IsReproducing())
		{
			sheepStateMachine->SetIsReproducing(true);
		}

		sheepStateMachine->SwitchState(std::make_unique<ReproducingState>(sheepStateMachine));
		return;
	}

	if (sheepStateMachine->IsGroupLeader())
	{
		if (movementTimer >= timeToChangeDirection)
		{
			moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f, static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
			movementTimer = 0.0f;
		}

		Vector2 currentPosition = sheepStateMachine->GetPosition();

		currentPosition.x += moveDirection.x * scaledSpeed * deltaTime;
		currentPosition.y += moveDirection.y * scaledSpeed * deltaTime;

		if (scaleFactor <= 0.5f && currentPosition.y < minimumY)
		{
			currentPosition.y = minimumY;
		}

		float sheepSize = ValueConfig::World::CellSize * scaleFactor;
		float rightMargin = 4.0f * scaleFactor;
		float bottomMargin = 4.0f * scaleFactor;

		float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
		float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

		currentPosition.x = std::max(0.0f, std::min(currentPosition.x, maxX));
		currentPosition.y = std::max(minimumY, std::min(currentPosition.y, maxY));

		sheepStateMachine->SetPosition(currentPosition);
	}
	else if (SheepStateMachine* leader = sheepStateMachine->GetGroupLeader())
	{
		if (movementTimer >= timeToChangeDirection * 0.7f)
		{
			moveDirection = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f, static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
			movementTimer = 0.0f;
		}

		Vector2 currentPosition = sheepStateMachine->GetPosition();
		Vector2 leaderPosition = leader->GetPosition();

		float dx = leaderPosition.x - currentPosition.x;
		float dy = leaderPosition.y - currentPosition.y;
		float distanceToLeader = static_cast<float>(sqrt(dx * dx + dy * dy));

		if (distanceToLeader > scaledMaxGroupDistance)
		{
			Vector2 toLeaderDirection = { 0, 0 };

			if (distanceToLeader > 0)
			{
				toLeaderDirection.x = dx / distanceToLeader;
				toLeaderDirection.y = dy / distanceToLeader;
			}

			Vector2 combinedDirection = { toLeaderDirection.x * 0.7f + moveDirection.x * 0.3f,toLeaderDirection.y * 0.7f + moveDirection.y * 0.3f };

			float combinedLength = static_cast<float>(sqrt(combinedDirection.x * combinedDirection.x + combinedDirection.y * combinedDirection.y));

			if (combinedLength > 0)
			{
				combinedDirection.x /= combinedLength;
				combinedDirection.y /= combinedLength;
			}

			currentPosition.x += combinedDirection.x * scaledSpeed * deltaTime;
			currentPosition.y += combinedDirection.y * scaledSpeed * deltaTime;
		}
		else if (distanceToLeader < scaledMinGroupDistance)
		{
			Vector2 awayFromLeaderDirection = { 0, 0 };

			if (distanceToLeader > 0)
			{
				awayFromLeaderDirection.x = -dx / distanceToLeader;
				awayFromLeaderDirection.y = -dy / distanceToLeader;
			}

			currentPosition.x += awayFromLeaderDirection.x * scaledSpeed * 0.5f * deltaTime;
			currentPosition.y += awayFromLeaderDirection.y * scaledSpeed * 0.5f * deltaTime;
		}
		else
		{
			currentPosition.x += moveDirection.x * scaledSpeed * 0.8f * deltaTime;
			currentPosition.y += moveDirection.y * scaledSpeed * 0.8f * deltaTime;
		}

		const auto& groupMembers = leader->GetGroupMembers();

		for (auto member : groupMembers)
		{
			if (member == sheepStateMachine)
			{
				continue;
			}

			Vector2 memberPosition = member->GetPosition();

			float memberDx = currentPosition.x - memberPosition.x;
			float memberDy = currentPosition.y - memberPosition.y;
			float memberDistance = static_cast<float>(sqrt(memberDx * memberDx + memberDy * memberDy));

			if (memberDistance < scaledMinGroupDistance)
			{
				Vector2 awayFromMemberDirection = { 0, 0 };

				if (memberDistance > 0)
				{
					awayFromMemberDirection.x = memberDx / memberDistance;
					awayFromMemberDirection.y = memberDy / memberDistance;
				}

				currentPosition.x += awayFromMemberDirection.x * scaledSpeed * 0.5f * deltaTime;
				currentPosition.y += awayFromMemberDirection.y * scaledSpeed * 0.5f * deltaTime;
			}
		}


		if (scaleFactor <= 0.5f && currentPosition.y < minimumY)
		{
			currentPosition.y = minimumY;
		}

		float sheepSize = ValueConfig::World::CellSize * scaleFactor;
		float rightMargin = 4.0f * scaleFactor;
		float bottomMargin = 4.0f * scaleFactor;

		float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
		float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

		currentPosition.x = std::max(0.0f, std::min(currentPosition.x, maxX));
		currentPosition.y = std::max(minimumY, std::min(currentPosition.y, maxY));

		sheepStateMachine->SetPosition(currentPosition);
	}
	else
	{
		sheepStateMachine->SwitchState(std::make_unique<WanderingAloneState>(sheepStateMachine));
	}
}

// Cleanup when exiting wandering in group state.
void WanderingInGroupState::Exit()
{
}

// Initializes the eating state with a zero consumption timer.
SheepEatingState::SheepEatingState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine), eatingTimer(0.0f), targetGrass(nullptr)
{
}

// Sets up eating behavior and updates sheep's visual state.
void SheepEatingState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		printf("Sheep entered EATING state\n");
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::Eating);
	eatingTimer = 0.0f;

	const auto& grasses = sheepStateMachine->GetWorld()->GetGrasses();
	const Vector2& sheepPosition = sheepStateMachine->GetPosition();

	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float scaledCellSize = ValueConfig::World::CellSize * scaleFactor;
	float minY = sheepStateMachine->GetWorld()->GetMinY();

	for (const auto& grass : grasses)
	{
		Vector2 grassPosition = CoordinateUtils::GridToWorldPosition(grass->GetPosition(), scaleFactor);

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

		float dx = sheepPosition.x - correctedGrassPosition.x;
		float dy = sheepPosition.y - correctedGrassPosition.y;
		float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

		if (distance < scaledCellSize / 2)
		{
			targetGrass = grass.get();
			break;
		}
	}
}

// Manages grass consumption and hunger/fullness updates.
void SheepEatingState::Tick(float deltaTime)
{
	eatingTimer += deltaTime;

	if (eatingTimer >= timeToEat)
	{
		bool foundGrass = false;

		float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
		float scaledCellSize = ValueConfig::World::CellSize * scaleFactor;
		float minY = sheepStateMachine->GetWorld()->GetMinY();

		if (targetGrass && targetGrass->IsAlive() && targetGrass->GetCurrentState() == GrassStateMachine::GrassState::FullyGrown)
		{
			targetGrass->Die();
			foundGrass = true;
		}
		else
		{
			const auto& grasses = sheepStateMachine->GetWorld()->GetGrasses();
			const Vector2& sheepPosition = sheepStateMachine->GetPosition();

			for (const auto& grass : grasses)
			{
				Vector2 grassPosition = CoordinateUtils::GridToWorldPosition(grass->GetPosition(), scaleFactor);

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

				float dx = sheepPosition.x - correctedGrassPosition.x;
				float dy = sheepPosition.y - correctedGrassPosition.y;
				float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

				if (distance < scaledCellSize / 2)
				{
					grass->Die();
					foundGrass = true;
					break;
				}
			}
		}

		if (foundGrass)
		{
			sheepStateMachine->SetHunger(0.0f);
			sheepStateMachine->SetFullness(RuntimeConfig::SheepMaxFullness());
		}

		if (sheepStateMachine->IsInGroup())
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingInGroupState>(sheepStateMachine));
		}
		else
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingAloneState>(sheepStateMachine));
		}
	}
}

// Cleanup when exiting eating state.
void SheepEatingState::Exit()
{
	if (targetGrass)
	{
		targetGrass->SetBeingEaten(false);
		targetGrass = nullptr;
	}
}

// Initializes the defecating state with a zero defecation timer.
DefecatingState::DefecatingState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine), defecatingTimer(0.0f)
{
}

// Sets up defecating behavior and updates sheep's visual state.
void DefecatingState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		printf("Sheep entered DEFECATING state\n");
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::Defecating);
	defecatingTimer = 0.0f;
}

// Manages defecation process and timer.
void DefecatingState::Tick(float deltaTime)
{
	defecatingTimer += deltaTime;

	if (defecatingTimer >= timeToDefecate)
	{
		sheepStateMachine->GetWorld()->AddSheepPoop(sheepStateMachine->GetPosition());

		sheepStateMachine->SetFullness(0.0f);

		if (sheepStateMachine->IsInGroup())
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingInGroupState>(sheepStateMachine));
		}
		else
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingAloneState>(sheepStateMachine));
		}
	}
}

// Cleanup when exiting defecating state.
void DefecatingState::Exit()
{
}

// Initializes the running away state for escaping from wolves.
RunningAwayState::RunningAwayState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine)
{
}

// Sets up fleeing behavior and updates sheep's visual state.
void RunningAwayState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		printf("Sheep entered RUNNING AWAY state\n");
	}

	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float scaledCellSize = ValueConfig::World::CellSize * scaleFactor;

	if (sheepStateMachine->GetCurrentState() == SheepStateMachine::SheepState::Eating)
	{
		const auto& grasses = sheepStateMachine->GetWorld()->GetGrasses();

		const Vector2& sheepPosition = sheepStateMachine->GetPosition();

		for (const auto& grass : grasses)
		{
			Vector2 grassPosition = CoordinateUtils::GridToWorldPosition(grass->GetPosition(), sheepStateMachine->GetWorld()->GetScaleFactor());

			float dx = sheepPosition.x - grassPosition.x;
			float dy = sheepPosition.y - grassPosition.y;
			float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

			if (distance < scaledCellSize / 2)
			{
				grass->SetBeingEaten(false);
				break;
			}
		}
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::RunningAway);

	randomDirectionOffset = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };

	escapeDirectionTimer = 0.0f;
}

// Manages wolf avoidance movement and detection.
void RunningAwayState::Tick(float deltaTime)
{
	escapeDirectionTimer += deltaTime;

	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float scaledFleeSpeed = fleeSpeed * scaleFactor;
	float scaledPanicDistance = ValueConfig::Sheep::PanicDistance * scaleFactor;
	float scaledWolfDetectionRadius = wolfDetectionRadius * scaleFactor;
	float minimumY = sheepStateMachine->GetWorld()->GetMinY();

	if (escapeDirectionTimer >= directionChangeInterval)
	{
		randomDirectionOffset = { static_cast<float>(GetRandomValue(-100, 100)) / 100.0f,static_cast<float>(GetRandomValue(-100, 100)) / 100.0f };
		escapeDirectionTimer = 0.0f;
	}

	Vector2 currentPosition = sheepStateMachine->GetPosition();
	Vector2 closestWolfPosition = { 0, 0 };

	const auto& wolves = sheepStateMachine->GetWorld()->GetWolves();

	float closestDistance = scaledWolfDetectionRadius;

	bool wolfFound = false;

	for (const auto& wolf : wolves)
	{
		Vector2 wolfPosition = wolf->GetPosition();

		float dx = currentPosition.x - wolfPosition.x;
		float dy = currentPosition.y - wolfPosition.y;
		float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

		if (distance < closestDistance)
		{
			closestDistance = distance;
			closestWolfPosition = wolfPosition;

			wolfFound = true;
		}
	}

	if (!wolfFound)
	{
		if (sheepStateMachine->IsInGroup())
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingInGroupState>(sheepStateMachine));
		}
		else
		{
			sheepStateMachine->SwitchState(std::make_unique<WanderingAloneState>(sheepStateMachine));
		}
		return;
	}

	if (wolfFound)
	{
		float dx = currentPosition.x - closestWolfPosition.x;
		float dy = currentPosition.y - closestWolfPosition.y;
		float distance = static_cast<float>(sqrt(dx * dx + dy * dy));

		Vector2 fleeDirection = { 0, 0 };

		if (distance > 0)
		{
			fleeDirection.x = dx / distance;
			fleeDirection.y = dy / distance;
		}

		fleeDirection.x += randomDirectionOffset.x * randomDirectionWeight;
		fleeDirection.y += randomDirectionOffset.y * randomDirectionWeight;

		float newLength = static_cast<float>(sqrt(fleeDirection.x * fleeDirection.x + fleeDirection.y * fleeDirection.y));

		if (newLength > 0)
		{
			fleeDirection.x /= newLength;
			fleeDirection.y /= newLength;
		}

		Vector2 edgeAvoidance = CalculateEdgeAvoidance(currentPosition);

		fleeDirection.x += edgeAvoidance.x * edgeAvoidanceWeight;
		fleeDirection.y += edgeAvoidance.y * edgeAvoidanceWeight;

		newLength = static_cast<float>((sqrt(fleeDirection.x * fleeDirection.x + fleeDirection.y * fleeDirection.y)));

		if (newLength > 0)
		{
			fleeDirection.x /= newLength;
			fleeDirection.y /= newLength;
		}

		float speedMultiplier = 1.0f;

		if (distance < scaledPanicDistance)
		{
			speedMultiplier = ValueConfig::Sheep::PanicSpeedMultiplier;
		}

		currentPosition.x += fleeDirection.x * scaledFleeSpeed * speedMultiplier * deltaTime;
		currentPosition.y += fleeDirection.y * scaledFleeSpeed * speedMultiplier * deltaTime;

		if (scaleFactor <= 0.5f && currentPosition.y < minimumY)
		{
			currentPosition.y = minimumY;
		}

		float sheepSize = ValueConfig::World::CellSize * scaleFactor;
		float rightMargin = 4.0f * scaleFactor;
		float bottomMargin = 4.0f * scaleFactor;

		float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
		float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

		currentPosition.x = std::max(0.0f, std::min(currentPosition.x, maxX));
		currentPosition.y = std::max(minimumY, std::min(currentPosition.y, maxY));

		sheepStateMachine->SetPosition(currentPosition);
	}
}

// Helper Function to Avoid getting trapped against screen edges.
Vector2 RunningAwayState::CalculateEdgeAvoidance(const Vector2& position)
{
	Vector2 avoidance = { 0, 0 };

	float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();
	float borderMargin = ValueConfig::Sheep::EdgeAvoidanceMargin * scaleFactor;

	float screenWidth = static_cast<float>(GetScreenWidth());
	float screenHeight = static_cast<float>(GetScreenHeight());

	if (position.x < borderMargin)
	{
		avoidance.x = 1.0f - (position.x / borderMargin);
	}
	else if (position.x > screenWidth - borderMargin)
	{
		avoidance.x = -1.0f * (1.0f - ((screenWidth - position.x) / borderMargin));
	}

	if (position.y < borderMargin)
	{
		avoidance.y = 1.0f - (position.y / borderMargin);
	}
	else if (position.y > screenHeight - borderMargin)
	{
		avoidance.y = -1.0f * (1.0f - ((screenHeight - position.y) / borderMargin));
	}

	return avoidance;
}

// Cleanup when exiting running away state.
void RunningAwayState::Exit()
{
	sheepStateMachine->SetGroupCooldownTimer(ValueConfig::Sheep::GroupCooldownAfterFleeing);
}

// Initializes the reproducing state with a zero reproduction timer.
ReproducingState::ReproducingState(SheepStateMachine* stateMachine) : SheepBaseState(stateMachine), reproducingTimer(0.0f)
{
}

// Sets up reproduction behavior and updates sheep's visual state.
void ReproducingState::Enter()
{
	if (Simulation* _simulation_ = dynamic_cast<Simulation*>(sheepStateMachine->GetWorld()->GetSimulation()))
	{
		_simulation_->AddConsoleMessage("Sheep entered REPRODUCING state\n");
	}

	sheepStateMachine->SetCurrentState(SheepStateMachine::SheepState::Reproducing);
	reproducingTimer = 0.0f;
}

// Manages reproduction process and creation of new sheep.
void ReproducingState::Tick(float deltaTime)
{
	reproducingTimer += deltaTime;

	if (reproducingTimer >= timeToReproduce)
	{
		if (sheepStateMachine->IsReproducing() && sheepStateMachine->IsGroupLeader())
		{
			int reproducingCount = sheepStateMachine->IsReproducing() ? 1 : 0;

			for (auto member : sheepStateMachine->GetGroupMembers())
			{
				if (member->IsReproducing())
				{
					reproducingCount++;
				}
			}

			int pairCount = reproducingCount / 2;

			float scaleFactor = sheepStateMachine->GetWorld()->GetScaleFactor();

			for (int i = 0; i < pairCount; i++)
			{
				Vector2 newSheepPosition = sheepStateMachine->GetPosition();

				newSheepPosition.x += static_cast<float>(GetRandomValue(-20, 20)) * scaleFactor;
				newSheepPosition.y += static_cast<float>(GetRandomValue(-20, 20)) * scaleFactor;

				World* world = sheepStateMachine->GetWorld();
				float minimumY = world->GetMinY();

				if (scaleFactor <= 0.5f && newSheepPosition.y < minimumY)
				{
					newSheepPosition.y = minimumY;
				}

				float sheepSize = ValueConfig::World::CellSize * scaleFactor;
				float rightMargin = 4.0f * scaleFactor;
				float bottomMargin = 4.0f * scaleFactor;

				float maxX = static_cast<float>(GetScreenWidth() * 3.0f / 4.0f - sheepSize - rightMargin);
				float maxY = static_cast<float>(GetScreenHeight() * 2.0f / 3.0f - sheepSize - bottomMargin);

				newSheepPosition.x = std::max(0.0f, std::min(newSheepPosition.x, maxX));
				newSheepPosition.y = std::max(minimumY, std::min(newSheepPosition.y, maxY));

				sheepStateMachine->RequestNewSheep(newSheepPosition);
			}

			for (auto member : sheepStateMachine->GetGroupMembers())
			{
				member->SetIsReproducing(false);

				if (member->GetCurrentState() == SheepStateMachine::SheepState::Reproducing)
				{
					member->SetCurrentState(SheepStateMachine::SheepState::WanderingInGroup);
				}
			}
		}

		sheepStateMachine->SetIsReproducing(false);

		sheepStateMachine->SwitchState(std::make_unique<WanderingInGroupState>(sheepStateMachine));
	}
}

// Cleanup when exiting reproducing state.
void ReproducingState::Exit()
{
	sheepStateMachine->ResetGroupTimer();
}
