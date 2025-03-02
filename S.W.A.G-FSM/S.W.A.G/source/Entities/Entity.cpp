#include <Entities/Entity.h>


// Initializes a new entity with its world reference and starting position.
Entity::Entity(World* world, Vector2 position) : world(world), position(position), isAlive(true), senseTimer(0.0f), decideTimer(0.0f)
{
}

// Updates entity's sense and decide timers, triggering respective actions at set intervals.
void Entity::Update(float deltaTime)
{
	senseTimer += deltaTime;
	decideTimer += deltaTime;

	if (senseTimer >= senseInterval)
	{
		Sense(deltaTime);
		senseTimer = 0.0f;
	}

	if (decideTimer >= decideInterval)
	{
		Decide(deltaTime);
		decideTimer = 0.0f;
	}

	Act(deltaTime);
}